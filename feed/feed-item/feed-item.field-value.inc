<?php

/**
 * Feed item field values formatter
 * 
 * @param $feed_item_field_values
 *   Array of feed item field value object.
 * @param $format
 *   A formatter type.
 */
function feed_item_field_values_format($feed_item_field_values, $format) {
  $result = array();
  
  foreach ($feed_item_field_values as $feed_item_field_value) {
    if ($formatted = feed_item_field_value_format($feed_item_field_value, $format)) {
      $result[$feed_item_field_value->fifvid] = $formatted;
    }
  }
  return $result;
}

/**
 * Feed item field values formatter
 * 
 * @param $feed_item_field_value
 *   A feed item field value object.
 * @param $format
 *   A formatter type.
 */
function feed_item_field_value_format($feed_item_field_value, $format) {
  $formatted = NULL;
  
  if ($feed_item_field = feed_field_load($feed_item_field_value->ffid)) {
    if ($info = feed_field_type_info($feed_item_field->type, TRUE)) {
      $field_settings = !empty($info['settings']) ? $info['settings'] : array();
      $field_instance = array_merge($field_settings, feed_object_get_data($feed_item_field, 'instance', array()));
      
      $formatter = feed_formatter_info($format);
      $formatter_settings = !empty($formatter['feed_field_setting_defaults']) ? $formatter['feed_field_setting_defaults'] : array();
      $formatter_instance = array_merge($formatter_settings, feed_object_get_data($feed_item_field, FEED_DATA_FEED_FIELD_DISPLAY_SETTINGS, array()));
      
      if (isset($info['feed_item_field_formatters'][$format])) {
        $function = (string) $info['feed_item_field_formatters'][$format];
        
        if (is_callable($function)) {
          $values = feed_object_get_data($feed_item_field_value, FEED_DATA_TYPE_FIELD_VALUE, array()) + $info['defaults'];
          
          $formatted = $function($feed_item_field, $feed_item_field_value, $field_instance, $formatter_instance, $values);
        }
      }
    }
  }
  return $formatted;
}

/**
 * Feed item field values apply
 * 
 * @param $feed_field
 *   A feed field object.
 * @param $feed_item_field_values
 *   Array of feed item field value object.
 * @param $values
 *   Array of values to apply indexed by fifvid if existing
 */
function feed_item_field_values_apply(&$feed_field, &$feed_item_field_values, $values) {
  $count = 1;
  
  foreach ($values as $fifvid => $value) {
    if (!isset($feed_item_field_values[$fifvid])) {
      $index = 'new_' . $count++;
      $feed_item_field_values[$index] = feed_item_field_value_defaults();
    }
    else {
      $index = $fifvid;
    }
    
    feed_field_value_apply($feed_field, $feed_item_field_values[$index], $value);
  }
}

/**
 * Feed item field value apply
 * 
 * @param $feed_item_field_value
 *   A feed item field value object.
 * @param $values
 *   Array of values to apply indexed by fifvid if existing
 */
function feed_item_field_value_apply(&$feed_field, &$feed_item_field_value, $values) {
  if ($info = feed_field_type_info($feed_field->type, TRUE)) {
    $settings = !empty($info['settings']) ? $info['settings'] : array();
    $instance = array_merge($settings, !empty($feed_field->data['instance']) ? $feed_field->data['instance'] : array());
    
    if (isset($info['submit_widget'])) {
      $function = (string) $info['submit_widget'];
      
      if (is_callable($function)) {
        $values = $function($feed_field, $instance, $values);
      }
    }
  
    if (!empty($values)) {
      if (!is_array($feed_item_field_value->data)) {
        $feed_item_field_value->data = array();
      }
      foreach (array_keys($info['defaults']) as $name) {
        if (isset($values[$name])) {
          $feed_item_field_value->data[FEED_DATA_TYPE_FIELD_VALUE][$name] = $values[$name];
        }
      }
    }
    else {
      $feed_item_field_value->delete = TRUE;
    }
  }
}

/**
 * Format a feed item field's value by tag name & namespace
 * 
 * @param $fiid
 *   A feed item id.
 * @param $tagname
 *   A string containing the tag name fo the field
 * @param $format
 *   The format type to format feed item field value.
 * @param $seperator
 *   A string chunk to join formatted feed item field values.
 */
function feed_item_field_values_tagname_format($fiid, $tagname, $format, $seperator = ' ') {
  $feed_item_field_values = feed_item_field_values_tagname_load($fiid, $tagname);
  
  $formatted = array();
  if (!empty($feed_item_field_values)) {
    foreach ($feed_item_field_values as $feed_item_field_value) {
      $formatted[] = feed_item_field_value_format($feed_item_field_value, $format);
    }
  }
  return implode($seperator, $formatted);
}

/**
 * Load a feed item field's values by tag name & namespace
 * 
 * @param $fiid
 *   A feed item id.
 * @param $tagname
 *   A string containing the tag name of the field
 * @param $reload
 *   A boolean to indicate whether to reload data from database.
 *   
 * @return
 *   An array containing feed item field value objects.
 */
function feed_item_field_values_tagname_load($fiid, $tagname, $reload = FALSE) {
  $result = array();
  $feed_item = feed_item_load($fiid);
  
  if (!empty($feed_item) && $feed_item_fields = feed_item_field_tagname_load($tagname, $feed_item->fid, $reload)) {
    foreach ($feed_item_fields as $ffid => $feed_item_field) {
      foreach (feed_item_field_values_load($feed_item, $feed_item_field, $reload) as $fifvid => $feed_item_field_value) {
        if (!isset($result[$fifvid])) {
          $result[$fifvid] = $feed_item_field_value;
        }
      }
    }
  }
  return $result;
}

/**
 * Load feed item field values. If a feed item field object is specified will 
 * return only those values for that feed item field
 *
 * @param $feed_item
 *   A feed item object.
 * @param $feed_field (optional)
 *   A feed field object.
 * @param $reload (optional)
 *   A boolean to indicate whether to reload data from database.
 *   
 * @return
 *   An array of feed item field value objects, indexed by ffvid and ffid. 
 *   If feed field is specified, this function will return an array of feed 
 *   item field value objects indexed by ffvid.
 */
function feed_item_field_values_load($feed_item, $feed_field = NULL, $reload = FALSE) {
  $feed_item_field_values_list = &drupal_static(__FUNCTION__);
  
  if (!isset($feed_item_field_values_list)) {
    $feed_item_field_values_list = array();
  }
  if ($reload || !isset($feed_item_field_values_list[$feed_item->fiid])) {
    $feed_item_field_values_list[$feed_item->fiid] = array();
    
    $feed_item_field_values = db_select('feed_item_field_value' , 'fifv')
      ->fields('fifv')
      ->condition('fifv.fiid', $feed_item->fiid)
      ->orderBy('fifv.fiid', 'DESC')
      ->execute()->fetchAllAssoc('fifvid');
      
    if (!empty($feed_item_field_values)) {
      $feed_item_field_value_datas = db_select('feed_item_field_value_data', 'fifvd')
        ->fields('fifvd')
        ->condition('fifvd.fifvid', array_keys($feed_item_field_values))
        ->execute()->fetchAll(PDO::FETCH_OBJ);
        
      foreach ($feed_item_field_value_datas as $feed_item_field_value_data) {
        $fifvid = $feed_item_field_value_data->fifvid;
        $name = $feed_item_field_value_data->name;
        $type = $feed_item_field_value_data->type;
        $data = $feed_item_field_value_data->data;
        
        if (isset($feed_item_field_values[$fifvid])) {
          if (!isset($feed_item_field_values[$fifvid]->data)) {
            $feed_item_field_values[$fifvid]->data = array();
          }
          if (!isset($feed_item_field_values[$fifvid]->data[$type])) {
            $feed_item_field_values[$fifvid]->data[$type] = array();
          }
          $feed_item_field_values[$fifvid]->data[$type][$name] = $data;
        }
      }
    }
    
    foreach ($feed_item_field_values as $feed_item_field_value) {
      $feed_item_field_values_list[$feed_item->fiid][$feed_item_field_value->ffid][$feed_item_field_value->fifvid] = $feed_item_field_value;
    }
  }
  
  if ($feed_field !== NULL) {
    if (isset($feed_field->ffid, $feed_item_field_values_list[$feed_item->fiid][$feed_field->ffid])) {
      return $feed_item_field_values_list[$feed_item->fiid][$feed_field->ffid];
    }
    return array();
  }
  else {
    return $feed_item_field_values_list[$feed_item->fiid];
  }
}

/**
 * Save feed item field values.
 *
 * @param $feed_item_field_values
 *   An array of feed item field values to save.
 * @param $feed_item
 *   A feed item object.
 * @param $feed_field
 *   A feed field object.
 * @param $transaction (optional)
 *   A DatabaseTransaction object.
 *   
 * @return
 *   Status flag indicating outcome of the operation.
 */
function feed_item_field_values_save(array &$feed_item_field_values, &$feed_item, $feed_field, DatabaseTransaction $transaction = NULL) {
  if (!$transaction) {
    $transaction = db_transaction();
  }
  
  foreach ($feed_item_field_values as &$feed_item_field_value) {
    if (!feed_item_field_value_save($feed_item_field_value, $feed_item, $feed_field, $transaction)) {
      return FALSE;
    }
  }
  return TRUE;
}

/**
 * Save feed item field value.
 *
 * @param $feed_item_field_value
 *   A feed item field value object to save.
 * @param $feed_item
 *   A feed item object.
 * @param $feed_field
 *   A feed field object.
 * @param $transaction (optional)
 *   A DatabaseTransaction object.
 *   
 * @return
 *   Status flag indicating outcome of the operation.
 */
function feed_item_field_value_save(&$feed_item_field_value, &$feed_item, $feed_field, DatabaseTransaction $transaction = NULL) {
  if (!$transaction) {
    $transaction = db_transaction();
  }

  try {
    // Marked to be deleted.
    if (!empty($feed_item_field_value->delete)) {
      if (isset($feed_item_field_value->fifvid)) {
        db_delete('feed_item_field_value_data')
          ->condition('fifvid', $feed_item_field_value->fifvid)
          ->execute();
          
        db_delete('feed_item_field_value')
          ->condition('fifvid', $feed_item_field_value->fifvid)
          ->execute();
      }
    }
    else {
      $feed_item_field_value->fiid = $feed_item->fiid;
      $feed_item_field_value->ffid = $feed_field->ffid;
      
      if (!$status = drupal_write_record('feed_item_field_value', $feed_item_field_value, isset($feed_item_field_value->fifvid) ? 'fifvid' : array())) {
        throw new Exception('Could not save a feed item field value record');
      }
      // Remove data values that do not exists anymore
      db_delete('feed_item_field_value_data')
        ->condition('fifvid', $feed_item_field_value->fifvid)
        ->execute();
        
      if (!empty($feed_item_field_value->data)) {
        $insert = db_insert('feed_item_field_value_data')->fields(array('fifvid', 'type', 'name', 'data'));
      
        foreach ($feed_item_field_value->data as $type => $data) {
          if (!is_array($data)) {
            throw new Exception('Invalid data value for feed item field value record');
          }
          
          foreach ($data as $name => $value) {
            $insert->values(array(
              'fifvid' => $feed_item_field_value->fifvid,
              'type' => $type,
              'name' => $name,
              'data' => $value,
            ));
          }
        }
        $insert->execute();
      }
      db_ignore_slave();
    }

    if ($feed_item->updated_at < REQUEST_TIME) {
      $feed_items = array(&$feed_item);
      feed_items_update_timestamp($feed_items);
    }
    return TRUE;
  }
  catch (Exception $e) {
    $transaction->rollback();
    watchdog_exception('feed', $e);
    return FALSE;
  }
}

/**
 * Delete feed item field value.
 *
 * @param $fifvids
 *   An array of feed item field value's fifvid to delete.
 *   
 * @return
 *   Status flag indicating outcome of the operation.
 */
function feed_item_field_values_delete(array $fifvids, DatabaseTransaction $transaction = NULL) {
  if (!$transaction) {
    $transaction = db_transaction();
  }

  try {
    foreach ($fifvids as &$fifvid) {
      if (is_object($fifvid) && isset($fifvid->fifvid)) {
        $fifvid = $fifvid->fifvid;
      }
    }
    // Remove data values
    db_delete('feed_item_field_value_data')
      ->condition('fifvid', $fifvids)
      ->execute();
      
    db_delete('feed_item_field_value')
      ->condition('fifvid', $fifvids)
      ->execute();
    
    return TRUE;
  }
  catch (Exception $e) {
    $transaction->rollback();
    watchdog_exception('feed', $e);
    return FALSE;
  }
}

/**
 * Configure feed item field value object/ensure defaults
 * 
 * @param $feed_item_field_value
 * 	 A feed item field value object.
 */
function feed_item_field_value_defaults($feed_item_field_value = NULL) {
  $feed_item_field_value = (object) $feed_item_field_value;
  if (!isset($feed_item_field_value->data)) $feed_item_field_value->data = array();
  
  return $feed_item_field_value;
}
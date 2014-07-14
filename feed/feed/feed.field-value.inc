<?php
/**
 * Feed field values formatter
 * 
 * @param $feed_field_values
 *   Array of feed field value objects.
 * @param $format
 *   A formatter type.
 */
function feed_field_values_format($feed_field_values, $format) {
  $result = array();
  
  foreach ($feed_field_values as $feed_field_value) {
    if ($formatted = feed_field_value_format($feed_field_value, $format)) {
      $result[$feed_field_value->ffvid] = $formatted;
    }
  }
  return $result;
}

/**
 * Feed item field values formatter
 * 
 * @param $feed_field_value
 *   A feed field value object.
 * @param $format
 *   A formatter type.
 */
function feed_field_value_format($feed_field_value, $format) {
  $formatted = NULL;
  
  if ($feed_field = feed_field_load($feed_field_value->ffid)) {
    if ($info = feed_field_type_info($feed_field->type, TRUE)) {
      $field_settings = !empty($info['settings']) ? $info['settings'] : array();
      $field_instance = array_merge($field_settings, feed_object_get_data($feed_field, 'instance', array()));
      
      $formatter = feed_formatter_info($format);
      $formatter_settings = !empty($formatter['feed_field_setting_defaults']) ? $formatter['feed_field_setting_defaults'] : array();
      $formatter_instance = array_merge($formatter_settings, feed_object_get_data($feed_field, FEED_DATA_FEED_FIELD_DISPLAY_SETTINGS, array()));
      
      if (isset($info['feed_field_formatters'][$format])) {
        $function = (string) $info['feed_field_formatters'][$format];
        
        if (is_callable($function)) {
          $values = feed_object_get_data($feed_field_value, FEED_DATA_TYPE_FIELD_VALUE, array()) + $info['defaults'];
          
          $formatted = $function($feed_field, $feed_field_value, $field_instance, $formatter_instance, $values);
        }
      }
    }
  }
  return $formatted;
}

/**
 * Feed field values apply
 * 
 * @param $feed_field_values
 *   Array of feed field value object.
 * @param $values
 *   Array of values to apply indexed by ffvid if existing
 */
function feed_field_values_apply(&$feed_field, &$feed_field_values, $values) {
  $count = 1;
  
  foreach ($values as $ffvid => $value) {
    if (!isset($feed_field_values[$ffvid])) {
      $index = 'new_' . $count++;
      $feed_field_values[$index] = feed_field_value_defaults();
    }
    else {
      $index = $ffvid;
    }
    
    feed_field_value_apply($feed_field, $feed_field_values[$index], $value);
  }
}

/**
 * Feed field value apply
 * 
 * @param $feed_field_value
 *   A feed field value object.
 * @param $values
 *   Array of values to apply indexed by ffvid if existing
 */
function feed_field_value_apply(&$feed_field, &$feed_field_value, $values) {
  if ($info = feed_field_type_info($feed_field->type, TRUE)) {
    $settings = !empty($info['settings']) ? $info['settings'] : array();
    $instance = array_merge($settings, !empty($feed_field->data['instance']) ? $feed_field->data['instance'] : array());
    
    if (isset($info['submit_widget'])) {
      $function = (string) $info['submit_widget'];
      
      if (function_exists($function)) {
        $values = $function($feed_field, $instance, $values);
      }
    }
  
    if (!empty($values)) {
      if (!is_array($feed_field_value->data)) {
        $feed_field_value->data = array();
      }
      foreach (array_keys($info['defaults']) as $name) {
        if (isset($values[$name])) {
          $feed_field_value->data[FEED_DATA_TYPE_FIELD_VALUE][$name] =  $values[$name];
        }
      }
    }
    else {
      $feed_field_value->delete = TRUE;
    }
  }
}

/**
 * Format a feed field's value by tag name & namespace
 * 
 * @param $fid
 *   A feed id.
 * @param $tagname
 *   A string containing the tag name fo the field
 * @param $format
 *   The format type to format feed field value.
 * @param $seperator
 *   A string chunk to join formatted feed field values.
 */
function feed_field_values_tagname_format($fid, $tagname, $format, $seperator = ' ') {
  $feed_field_values = feed_field_values_tagname_load($fid, $tagname);
  
  $formatted = array();
  if (!empty($feed_field_values)) {
    foreach ($feed_field_values as $feed_field_value) {
      $formatted[] = feed_field_value_format($feed_field_value, $format);
    }
  }
  return implode($seperator, $formatted);
}

/**
 * Load a feed field's values by tag name & namespace
 * 
 * @param $fid
 *   A feed id.
 * @param $tagname
 *   A string containing the tag name of the field
 * @param $reload
 *   A boolean to indicate whether to reload data from database.
 *   
 * @return
 *   An array containing feed field value objects.
 */
function feed_field_values_tagname_load($fid, $tagname, $reload = FALSE) {
  $result = array();
  if ($feed_fields = feed_field_tagname_load($tagname, $fid, $reload)) {
    foreach ($feed_fields as $ffid => $feed_field) {
      if ($feed = feed_load($feed_field->fid)) {
        foreach (feed_field_values_load($feed, $feed_field, $reload) as $ffvid => $feed_field_value) {
          if (!isset($result[$ffvid])) {
            $result[$ffvid] = $feed_field_value;
          }
        }
      }
    }
  }
  return $result;
}

/**
 * Load feed field values. If a feed field object is specified will return only 
 * those values for that feed field
 *
 * @param $feed
 *   A feed object.
 * @param $feed_field (optional)
 *   A feed field object.
 * @param $reload (optional)
 *   Boolean to indicate if request should be reloaded from database.
 *   
 * @return
 *   An array of feed field value objects, indexed by ffvid and ffid. 
 *   If feed field is specified, this function will return an array of feed 
 *   field value objects indexed by ffvid.
 */
function feed_field_values_load($feed, $feed_field = NULL, $reload = FALSE) {
  $feed_field_values_list = &drupal_static(__FUNCTION__);
  
  if (!isset($feed_field_values_list)) {
    $feed_field_values_list = array();
  }
  if ($reload || !isset($feed_field_values_list[$feed->fid])) {
    $feed_field_values_list[$feed->fid] = array();
    
    $feed_field_values = db_select('feed_field_value' , 'ffv')
      ->fields('ffv', array('ffvid', 'fid', 'ffid'))
      ->condition('ffv.fid', $feed->fid)
      ->orderBy('ffv.fid', 'DESC')
      ->execute()->fetchAllAssoc('ffvid');
      
    if (!empty($feed_field_values)) {
      $feed_field_value_datas = db_select('feed_field_value_data', 'ffvd')
        ->fields('ffvd')
        ->condition('ffvd.ffvid', array_keys($feed_field_values))
        ->execute()->fetchAll(PDO::FETCH_OBJ);
        
      foreach ($feed_field_value_datas as $feed_field_value_data) {
        $ffvid = $feed_field_value_data->ffvid;
        $name = $feed_field_value_data->name;
        $type = $feed_field_value_data->type;
        $data = $feed_field_value_data->data;
        
        if (isset($feed_field_values[$ffvid])) {
          if (!isset($feed_field_values[$ffvid]->data)) {
            $feed_field_values[$ffvid]->data = array();
          }
          if (!isset($feed_field_values[$ffvid]->data[$type])) {
            $feed_field_values[$ffvid]->data[$type] = array();
          }
          $feed_field_values[$ffvid]->data[$type][$name] = $data;
        }
      }
    }
    
    foreach ($feed_field_values as $feed_field_value) {
      $feed_field_values_list[$feed->fid][$feed_field_value->ffid][$feed_field_value->ffvid] = $feed_field_value;
    }
  }
  
  if ($feed_field !== NULL) {
    if (isset($feed_field->ffid, $feed_field_values_list[$feed->fid][$feed_field->ffid])) {
      return $feed_field_values_list[$feed->fid][$feed_field->ffid];
    }
    return array();
  }
  else {
    return $feed_field_values_list[$feed->fid];
  }
}

/**
 * Save feed field values.
 *
 * @param $feed_field_values
 *   An array of feed field values to save.
 * @param $feed
 *   A feed object.
 * @param $feed_field
 *   A feed field object.
 * @param $transaction (optional)
 *   A DatabaseTransaction object.
 *   
 * @return
 *   Status flag indicating outcome of the operation.
 */
function feed_field_values_save(array &$feed_field_values, $feed, $feed_field, DatabaseTransaction $transaction = NULL) {
  if (!$transaction) {
    $transaction = db_transaction();
  }
  
  foreach ($feed_field_values as &$feed_field_value) {
    if (!feed_field_value_save($feed_field_value, $feed, $feed_field, $transaction)) {
      return FALSE;
    }
  }
  return TRUE;
}

/**
 * Save feed field value.
 *
 * @param $feed_field_value
 *   A feed field value object to save.
 * @param $feed
 *   A feed object.
 * @param $feed_field
 *   A feed field object.
 * @param $transaction (optional)
 *   A DatabaseTransaction object.
 *   
 * @return
 *   Status flag indicating outcome of the operation.
 */
function feed_field_value_save(&$feed_field_value, $feed, $feed_field, DatabaseTransaction $transaction = NULL) {
  if (!$transaction) {
    $transaction = db_transaction();
  }

  try {
    // Marked to be deleted.
    if (!empty($feed_field_value->delete)) {
      if (isset($feed_field_value->ffvid)) {
        db_delete('feed_field_value_data')
          ->condition('ffvid', $feed_field_value->ffvid)
          ->execute();
          
        db_delete('feed_field_value')
          ->condition('ffvid', $feed_field_value->ffvid)
          ->execute();
      }
    }
    else {
      $feed_field_value->fid = $feed->fid;
      $feed_field_value->ffid = $feed_field->ffid;
      
      if (!$status = drupal_write_record('feed_field_value', $feed_field_value, isset($feed_field_value->ffvid) ? 'ffvid' : array())) {
        throw new Exception('Could not save a feed field value record');
      }
        
      // Remove data values that do not exists anymore
      db_delete('feed_field_value_data')
        ->condition('ffvid', $feed_field_value->ffvid)
        ->execute();
        
      if (!empty($feed_field_value->data)) {
        $insert = db_insert('feed_field_value_data')->fields(array('ffvid', 'type', 'name', 'data'));
        
        foreach ($feed_field_value->data as $type => $data) {
          if (!is_array($data)) {
            throw new Exception('Invalid data value for feed field value record');
          }
          
          foreach ($data as $name => $value) {
            $insert->values(array(
              'ffvid' => $feed_field_value->ffvid,
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
    return TRUE;
  }
  catch (Exception $e) {
    $transaction->rollback();
    watchdog_exception('feed', $e);
    return FALSE;
  }
}
/**
 * Delete feed field value.
 *
 * @param $fifvids
 *   An array of feed field value's fifvid to delete.
 *   
 * @return
 *   Status flag indicating outcome of the operation.
 */
function feed_field_values_delete(array $ffvids, DatabaseTransaction $transaction = NULL) {
  if (!$transaction) {
    $transaction = db_transaction();
  }

  try {
    foreach ($ffvids as &$ffvid) {
      if (is_object($ffvid) && isset($ffvid->ffvid)) {
        $ffvid = $ffvid->ffvid;
      }
    }
    // Remove data values
    db_delete('feed_field_value_data')
      ->condition('ffvid', $ffvids)
      ->execute();
      
    db_delete('feed_field_value')
      ->condition('ffvid', $ffvids)
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
 * Configure feed field value object/ensure defaults
 * 
 * @param $feed_field_value
 * 	 A feed field value object.
 */
function feed_field_value_defaults($feed_field_value = NULL) {
  $feed_field_value = (object) $feed_field_value;
  if (!isset($feed_field_value->data)) $feed_field_value->data = array();
  
  return $feed_field_value;
}
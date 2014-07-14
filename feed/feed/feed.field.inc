<?php

/**
 * Builds an field tree
 * 
 * @param $feed_fields
 *   An array of feed fields.
 *   
 * @return
 *   An array representing the field tree.
 */
function feed_field_build_tree(array $feed_fields) {
  $indexes = array('ffid' => array(), 'pffid' => array(), 'tree' => array());
  
  foreach ($feed_fields as &$feed_field) {
    $indexes['ffid'][$feed_field->ffid] =& $feed_field;
    
    if (!isset($indexes['pffid'][$feed_field->pffid])) {
      $indexes['pffid'][$feed_field->pffid] = array();
    }
    $indexes['pffid'][$feed_field->pffid][] =& $feed_field;
  }
  
  $root = array();
  foreach ($indexes['pffid'] as $pffid => $list) {
    if (!isset($indexes['ffid'][$pffid])) {
      $root += $list;
    }
  }
  
  $return = array();
  _feed_field_build_tree_depth($return, $root, $indexes);
  
  return $return;
}

/**
 * Function used by uasort to sort feed fields by weight.
 */
function feed_fields_sort($a, $b) {
  $a_weight = (is_object($a) && isset($a->data['weight'])) ? $a->data['weight'] : 0;
  $b_weight = (is_object($b) && isset($b->data['weight'])) ? $b->data['weight'] : 0;
  if ($a_weight == $b_weight) {
    return 0;
  }
  return ($a_weight < $b_weight) ? -1 : 1;
}

/**
 * Internal function that builds and marks the feed field depth
 * 
 * @see feed_field_build_tree().
 */
function _feed_field_build_tree_depth(&$tree, array $feed_fields, array &$indexes, $depth = 0) {
  uasort($feed_fields, 'feed_fields_sort');

  foreach ($feed_fields as $feed_field) {
    $tree[] = $feed_field;
    $feed_field->depth = $depth;
    
    if (isset($indexes['pffid'][$feed_field->ffid])) {
      $feed_field_children = $indexes['pffid'][$feed_field->ffid];
      unset($indexes['pffid'][$feed_field->ffid]);
      
      _feed_field_build_tree_depth($tree, $feed_field_children, $indexes, $depth + 1);
    }
  }
}

/**
 * Theming function; Creates feed field table.
 */
function theme_feed_field_attach($variables) {
  $elements = $variables['elements'];
  
  // Headers.
  $header = array(
    array('data' => 'Label'),
    array('data' => 'Weight'),
    array('data' => 'Tag'),
    array('data' => 'Type'),
    array('data' => 'Operations'),
  );
  
  $rows = array();
  
  // Field Rows. 
  if (!empty($elements['fields'])) {
    foreach (element_children($elements['fields'], TRUE) as $name) {
      $field = $elements['fields'][$name];
      
      $rows[] = array(
        'class' => array('draggable'),
        'data' => array(
          array(
          	'data' => !empty($field['#depth']) ? array(array(
            	'#type' => 'markup', 
            	'#markup' => theme('indentation', array('size' => $field['#depth'])),
            ), $field['name']) : array($field['name'])),
          array('data' => array($field['weight'], $field['parent'], $field['id'])),
          array('data' => $field['tag']),
          array('data' => $field['type']),
          array('data' => $field['settings']),
        ),
      );
    }
  }
  
  if (!empty($elements['add-field'])) {
    // New field Rows.
    $row = array();
    $row[] = array('data' => $elements['add-field']['name']);
    $row[] = array('data' => array($elements['add-field']['weight'], $elements['add-field']['parent']));
    $row[] = array('data' => $elements['add-field']['tag']);
    $row[] = array('data' => $elements['add-field']['type']);
    $row[] = array('data' => '&nbsp;');
    
    $rows[] = array('data' => $row, 'class' => array('draggable', 'tabledrag-leaf'));
  }
  
  drupal_add_tabledrag('feed-field', 'match', 'parent', 'field-parent', 'field-parent', 'field-id');
  drupal_add_tabledrag('feed-field', 'order', 'sibling', 'field-weight');
  
  return theme('feed_block', array(
    'attributes' => array('id' => 'feed-field-container'),
    'title' => 'Feed Fields',
    'content' => array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
      '#attributes' => array(
      	'id' => 'feed-field',
      ),
    ),
  ));
}

/**
 * Create a separate form element for each field.
 * 
 * @param $form
 * @param $form_state
 * @param $feed
 *   A feed object.
 */
function feed_field_overview_attach(&$form, &$form_state) {
  $feed = $form_state['#feed'];
  
  $form['feed_field'] = array();
  $form['feed_field']['#type'] = 'feed_field_attach';
  $form['feed_field']['#tree'] = TRUE;
  
  if (!empty($feed->fid)) {
    $feed_fields = feed_load_fields($feed, FALSE);
    $feed_fields = feed_field_build_tree($feed_fields);
    
    foreach ($feed_fields as $feed_field) {
      if (!isset($form['feed_field']['fields'])) {
        $form['feed_field']['fields'] = array('#tree' => TRUE);
      }
      
      $form['feed_field']['fields'][$feed_field->ffid] = array(
        '#tree' => TRUE, 
        '#depth' => $feed_field->depth,
        '#weight' => isset($feed_field->data['weight']) ? $feed_field->data['weight'] : 0,
      );
      
      $form['feed_field']['fields'][$feed_field->ffid]['id'] = array(
        '#type' => 'hidden',
        '#value' => !empty($feed_field->ffid) ? $feed_field->ffid : NULL,
        '#attributes' => array(
          'class' => array('field-id'),
        ),
      );
      $form['feed_field']['fields'][$feed_field->ffid]['name'] = array(
        '#type' => 'markup',
        '#markup' => !empty($feed_field->name) ? $feed_field->name : 'No Label',
        '#title_display' => 'invisible',
        '#title' => t('Label'),
        '#attributes' => array('class' => array('field-name')),
      );
      $form['feed_field']['fields'][$feed_field->ffid]['weight'] = array(
        '#type' => 'weight',
        '#delta' => 50,
        '#default_value' => isset($feed_field->data['weight']) ? $feed_field->data['weight'] : 0,
        '#title_display' => 'invisible',
        '#title' => t('Weight for field'),
        '#attributes' => array('class' => array('field-weight')),
      );
      $form['feed_field']['fields'][$feed_field->ffid]['parent'] = array(
        '#type' => 'hidden',
        '#default_value' => !empty($feed_field->pffid) ? $feed_field->pffid : NULL,
        '#attributes' => array(
          'class' => array('field-parent'),
        ),
      );
      $form['feed_field']['fields'][$feed_field->ffid]['tag'] = array(
        '#type' => 'hidden',
        '#prefix' => !empty($feed_field->tag) ? $feed_field->tag : 'No Tag',
        '#value' => !empty($feed_field->tag) ? $feed_field->tag : '',
        '#attributes' => array('class' => array('field-tag')),
      );
      $form['feed_field']['fields'][$feed_field->ffid]['type'] = array(
        '#type' => 'markup',
        '#markup' => !empty($feed_field->type) ? $feed_field->type : '',
      );
      $form['feed_field']['fields'][$feed_field->ffid]['settings'] = array(
        '#type' => 'link',
        '#title' => t('Settings'),
        '#href' => 'feed/' . $feed->fid . '/fields/' . $feed_field->ffid,
        '#options' => array('attributes' => array('title' => t('Edit feed field settings.'))),
      );
    }
  }
  
  // Add New field attribute.
  $form['feed_field']['add-field'] = array('#tree' => TRUE);
  $form['feed_field']['add-field']['name'] = array(
    '#type' => 'textfield',
    '#title' => t('New field label'),
    '#title_display' => 'invisible',
    '#size' => 15,
    '#description' => t('Feed field label'),
  );
  $form['feed_field']['add-field']['weight'] = array(
    '#type' => 'weight',
    '#delta' => 50,
    '#default_value' => 0,
    '#title_display' => 'invisible',
    '#title' => t('Weight for new field'),
    '#description' => t('Weight'),
    '#attributes' => array(
      'class' => array('field-weight'),
    ),
  );
  $form['feed_field']['add-field']['parent'] = array(
    '#type' => 'hidden',
    '#default_value' => '',
    '#title_display' => 'invisible',
    '#attributes' => array(
      'class' => array('field-parent'),
    ),
  );
  $form['feed_field']['add-field']['tag'] = array(
    '#type' => 'textfield',
    '#title' => t('New feed field tag'),
    '#title_display' => 'invisible',
    '#description' => t('XML tag name of this field'),
  );
  $form['feed_field']['add-field']['type'] = array(
    '#type' => 'select',
    '#title' => t('Type of new field'),
    '#title_display' => 'invisible',
    '#options' => feed_field_type_options(),
    '#empty_option' => t('- Select a type -'),
    '#description' => t('Type of field.'),
    '#attributes' => array('class' => array('field-type')),
  );
}

/**
 * Submit handler for feed_field_attach() elements.
 */
function feed_field_overview_attach_submit($form, &$form_state) {
  $feed =& $form_state['#feed'];
  
  if (isset($feed->fid)) {
    $feed_fields = feed_load_fields($feed, FALSE);
    $feed_fields_stack = array();
    
    if (!empty($feed_fields)) {
      foreach ($feed_fields as $feed_field) {
        if (isset($form_state['values']['feed_field']['fields'][$feed_field->ffid])) {
          $values = $form_state['values']['feed_field']['fields'][$feed_field->ffid];
          
          if (isset($values['weight'])) {
            $feed_field->data['weight'] = $values['weight'];
          }
          if (isset($feed_fields[$values['parent']])) {
            $feed_field->pffid = $values['parent']; // @todo write validation
          }
          elseif (empty($values['parent'])) {
            $feed_field->pffid = 0;
          }
          $feed_fields_stack[$feed_field->ffid] = $feed_field;
        }
      }
    }
  }
  
  if (!empty($form_state['values']['feed_field']['add-field']['tag'])) {
    $feed_field_values = $form_state['values']['feed_field']['add-field'];
    $feed_field = feed_field_defaults();
    
    $info = feed_field_type_info($feed_field_values['type']);
    
    if (isset($feed_fields[$feed_field_values['parent']])) {
      $feed_field->pffid = $feed_field_values['parent']; // @todo write validation
    }
    $feed_field->name = $feed_field_values['name'];
    $feed_field->tag = $feed_field_values['tag'];
    $feed_field->type = $feed_field_values['type'];
    $feed_field->module = $info['module'];
    $feed_field->fid = $feed->fid;
    
    $feed_fields_stack['add-field'] = $feed_field;
  }
  
  if (!empty($feed_fields_stack)) {
    $status = feed_fields_save($feed_fields_stack, $feed, FALSE);
    
    if ($status && !empty($feed_fields_stack['add-field'])) {
      $form_state['redirect'] = 'feed/' . $feed->fid . '/fields/' . $feed_fields_stack['add-field']->ffid;
    }
  }
}

/**
 * Create feed field settings elements
 */
function feed_field_settings_attach(&$form, &$form_state) {
  $feed = $form_state['#feed'];
  $feed_field = $form_state['#feed_field'];

  if ($info = feed_field_type_info($feed_field->type, TRUE)) {
    if (isset($info['form_settings'])) {
      $settings = !empty($info['settings']) ? $info['settings'] : array();
      $instance = array_merge($settings, feed_object_get_data($feed_field, 'instance', array()));
      
      $function = (string) $info['form_settings'];
      
      if (function_exists($function)) {
        $elements = $function($feed_field, $settings, $instance);
      
        if (is_array($elements) && !empty($elements)) {
          if (!isset($form['feed_field_settings'])) {
            $form['feed_field_settings'] = array(
            	'#tree' => TRUE,
              '#type' => 'feed_fieldset',
              '#title' => t('Feed Field Settings'),
            );
          }
          
          if (!isset($form['feed_field_settings'][$feed_field->fid])) {
            $form['feed_field_settings'][$feed_field->fid] = array();
          }
          $form['feed_field_settings'][$feed_field->fid] += $elements;
        }
      }
    }
  }
}

/**
 * Submit handler for feed_field_settings_attach() elements.
 */
function feed_field_settings_attach_submit(&$form, &$form_state) {
  $feed_field = $form_state['#feed_field'];
  
  if (!empty($form_state['values']['feed_field_settings'])) {
    feed_field_settings_apply($feed_field, $form_state['values']['feed_field_settings'][$feed_field->fid]);
  }
}

/**
 * Apply feed field setting values to the feed field.
 * 
 * @param $feed_field
 *   A feed field object.
 * @param $values
 *   Array of values to apply.
 */
function feed_field_settings_apply(&$feed_field, $values = NULL) {
  if ($info = feed_field_type_info($feed_field->type, TRUE)) {
    $settings = !empty($info['settings']) ? $info['settings'] : array();
    $instance = array_merge($settings, feed_object_get_data($feed_field, 'instance', array()));
    
    if (isset($info['submit_settings'])) {
      $function = (string) $info['submit_settings'];
      
      if (function_exists($function)) {
        $values = $function($feed_field, $instance, $values);
      }
    }
    
    $data = array();
    foreach ($settings as $name => $default) {
      $data[$name] = isset($values[$name]) ? $values[$name] : $default;
    }
    feed_object_set_data($feed_field, 'instance', $data);
  }
}

/**
 * Form process element for a feed field widget.
 */
function form_process_feed_field_widget(&$element, &$form_state) {
  if (!$info = feed_field_type_info($element['#feed_field']->type, TRUE)) {
    return FALSE;
  }
  $feed_field = &$element['#feed_field'];
  $feed_field_values = &$element['#feed_field_values'];
  
  $element['widget'] = array();
  $element['widget']['#type'] = 'container';
  $element['widget']['#tree'] = TRUE;
  
  if (empty($feed_field_values)) {
    $feed_field_values = array('new' => feed_field_value_defaults());
  }
  
  if (isset($info['form_widget'])) {
    $settings = !empty($info['settings']) ? $info['settings'] : array();
    $instance = array_merge($settings, feed_object_get_data($feed_field, 'instance', array()));
    
    $function = (string) $info['form_widget'];
  
    if (!empty($feed_item_field->is_required)) {
      $instance['required'] = TRUE;
    }
    
    foreach ($feed_field_values as $ffvid => $feed_field_value) {
      $values = feed_object_get_data($feed_field_value, FEED_DATA_TYPE_FIELD_VALUE, array());
      $element['widget'][$ffvid] = $function($feed_field, $instance, $values);
    }
  }
  return $element;
}

/**
 * Form submit handler to save feed field widget.
 */
function form_validate_feed_field_widget(&$element, &$form_state) {
  $feed_field = &$element['#feed_field'];
  $feed_field_values = &$element['#feed_field_values'];
  
  $values = drupal_array_get_nested_value($form_state['values'], $element['#parents']);
  
  if (!empty($values)) {
    feed_field_values_apply($feed_field, $feed_field_values, $values['widget']);
  }
}

/**
 * Load a feed's field by tag name & namespace
 * 
 * @param $tagname
 *   A string containing the tag name of the field
 * @param $fid (optional)
 *   A feed id that the field belongs.
 * @param $reload (optional)
 *   A boolean to indicate whether to reload data from database.
 *   
 * @return
 *   A feed field object.
 */
function feed_field_tagname_load($tagname, $fid = NULL, $reload = FALSE) {
  $feed_fields = &drupal_static(__FUNCTION__, array());
  
  if (!isset($feed_fields[$tagname])) {
    $feed_fields[$tagname] = array();
  }
  if ($reload || empty($feed_fields[$tagname])) {
    if (strpos($tagname, ':') === FALSE) {
      $tag = $tagname;
    }
    else {
      list($tag, $namespace) = explode(':', $tagname, 2);
    }
    
    $query = db_select('feed_field', 'ff')
      ->fields('ff', array('ffid'))
      ->condition('ff.is_feed_item_field', FALSE)
      ->condition('ff.tag', $tag);
      
    if (!empty($namespace)) {
      $query->condition('ff.namespace', $namespace);
    }
    $feed_fields[$tagname] = $query->execute()->fetchAllKeyed(0, 0);
  }

  if (!empty($fid)) {
    $result = array();
    
    if ($feed = feed_load($fid, $reload)) {
      foreach (feed_load_fields($feed, FALSE, $reload) as $field) {
        if (in_array($field->ffid, $feed_fields[$tagname])) {
          $result[$field->ffid] = $field;
        }
      }
    }
    return $result;
  }
  return feed_fields_load($feed_fields[$tagname], $reload);
}
  
/**
 * Load a feed field by ffid.
 *
 * @param $ffid
 *   A feed field ffid.
 * @param $reload (optional)
 *   A boolean to indicate whether to reload data from database.
 *   
 * @return
 *   A feed field object.
 */
function feed_field_load($ffid, $reload = FALSE) {
  return end(feed_fields_load(array($ffid), $reload));
}

/**
 * Load feed fields by ffids.
 *
 * @param $ffids
 *   An array of feed field ffids.
 * @param $reload (optional)
 *   A boolean to indicate whether to reload data from database.
 *   
 * @return
 *   An array of feed field objects.
 */
function feed_fields_load(array $ffids, $reload = FALSE) {
  $feed_fields = &drupal_static(__FUNCTION__, array());

  if ($reload) {
    $diff_ffids = $ffids;
  }
  else {
    $diff_ffids = array_diff($ffids, array_keys($feed_fields));
  }
  
  if (!empty($diff_ffids)) {
    $diff_feed_fields = db_select('feed_field', 'ff')
      ->fields('ff')
      ->condition('ff.ffid', $diff_ffids)
      ->execute()->fetchAllAssoc('ffid');
  
    foreach ($diff_feed_fields as $diff_feed_field) {
      $feed_fields[$diff_feed_field->ffid] = feed_field_attach_loaded($diff_feed_field);
    }
  }

  $results = array();
  foreach ($ffids as $ffid) {
    if (isset($feed_fields[$ffid])) {
      $results[$feed_fields[$ffid]->ffid] = $feed_fields[$ffid];
    }
  }
  return $results;
}

/**
 * Load subscribed feed fields 
 * 
 * @param $feed_subscription
 *   A feed object.
 * @param $reload (optional)
 *   A boolean to indicate whether to reload data from database.
 * 
 * @return
 *   An array of feed field objects, indexed by ffid.
 */
function feed_load_subscription_fields($feed_subscription, $reload = FALSE) {
  $feed_subscription_fields = &drupal_static(__FUNCTION__, array());
  
  if ($reload || !isset($feed_subscription_fields[$feed_subscription->fid])) {
    $query = db_select('feed_item_subscription', 'fis');
    $query->join('feed_item', 'fi', 'fi.fiid = fis.fiid');
    $query->join('feed_field', 'ff', 'ff.fid = fi.fid');
    $query->fields('fis', array('fiid'));
    $query->condition('fis.fid', $feed_subscription->fid);
    
    $feed_subscription_fields[$feed_subscription->fid] = $query->execute()->fetchAllAssoc('ffid');
  }
  return $feed_subscription_fields[$feed_subscription->fid];
}

/**
 * Load feed fields.
 *
 * @param $feed
 *   A feed object.
 * @param $is_feed_item_field
 *   A boolean to indicate which type of fields to load. TRUE will load
 *   the feed's feed item fields and FALSE will load the feed's attibutes
 * @param $reload (optional)
 *   A boolean to indicate whether to reload data from database.
 *   
 * @return
 *   An array of feed field objects, indexed by ffid.
 */
function feed_load_fields($feed, $is_feed_item_field = FALSE, $reload = FALSE) {
  $feed_fields = &drupal_static(__FUNCTION__);
  $is_feed_item_field = empty($is_feed_item_field) ? '0' : '1';
  
  if (!isset($feed_fields)) {
    $feed_fields = array();
  }
  if (!isset($feed_fields[$feed->fid])) {
    $feed_fields[$feed->fid] = array();
  }
  if ($reload || !isset($feed_fields[$feed->fid][$is_feed_item_field])) {
    $feed_fields[$feed->fid][$is_feed_item_field] = array();
    
    $ffids = db_select('feed_field', 'ff')
      ->fields('ff', array('ffid'))
      ->condition('ff.fid', $feed->fid)
      ->condition('ff.is_feed_item_field', $is_feed_item_field)
      ->execute()->fetchAllKeyed(0, 0);
      
    $feed_fields[$feed->fid][$is_feed_item_field] = feed_fields_load($ffids, $reload);
  }

  return $feed_fields[$feed->fid][$is_feed_item_field];
}

/**
 * Load feed fields from a feed item.
 *
 * @param $feed_item
 *   A feed item object.
 * @param $reload (optional)
 *   A boolean to indicate whether to reload data from database.
 *   
 * @return
 *   An array of feed field objects, indexed by ffid.
 */
function feed_item_fields_load($feed_item, $reload = FALSE) {
  if ($feed = feed_load($feed_item->fid, $reload)) {
    return feed_load_fields($feed, TRUE, $reload);
  }
  return array();
}

/**
 * Attach attributes and unserialize data in feed field
 * 
 * @param $feed_field
 *   A feed field object.
 */
function feed_field_attach_loaded($feed_field) {
  $feed_field = feed_field_defaults($feed_field);
  
  if (!empty($feed_field->data) && is_string($feed_field->data)) {
    $feed_field->data = @unserialize($feed_field->data);
  }
  return $feed_field;
}

/**
 * Save feed fields.
 *
 * @param $feed_fields
 *   An array of feed fields to save.
 * @param $feed
 *   A feed object.
 * @param $is_feed_item_field
 *   A boolean to indicate which type to save fields as. TRUE will save
 *   the fields as feed item fields and FALSE will save the fields
 *   as feed attibutes.
 * @param $transaction (optional)
 *   A DatabaseTransaction object.
 *   
 * @return
 *   Status flag indicating outcome of the operation.
 */
function feed_fields_save(array &$feed_fields, $feed, $is_feed_item_field = FALSE, DatabaseTransaction $transaction = NULL) {
  if (!$transaction) {
    $transaction = db_transaction();
  }

  try {
    $is_feed_item_field = ($is_feed_item_field === FALSE) ? '0' : '1';
    foreach ($feed_fields as &$feed_field) {
      $feed_field->is_feed_item_field = $is_feed_item_field;
      $feed_field->fid = $feed->fid;
      
      if (!$status = drupal_write_record('feed_field', $feed_field, isset($feed_field->ffid) ? 'ffid' : array())) {
        throw new Exception('Could not save a feed field record');
      }
    }
    db_ignore_slave();
    
    return TRUE;
  }
  catch (Exception $e) {
    $transaction->rollback();
    watchdog_exception('feed', $e);
    return FALSE;
  }
}

/**
 * Save feed field.
 *
 * @param $feed_field
 *   A feed field object to save.
 * @param $feed
 *   A feed object.
 * @param $is_feed_item_field
 *   A boolean to indicate which type to save this field as. TRUE will save
 *   the field as a feed item field and FALSE will save the field
 *   as a feed field
 * @param $transaction (optional)
 *   A DatabaseTransaction object.
 *   
 * @return
 *   Status flag indicating outcome of the operation.
 */
function feed_field_save(&$feed_field, $feed, $is_feed_item_field = FALSE, DatabaseTransaction $transaction = NULL) {
  if (!$transaction) {
    $transaction = db_transaction();
  }

  try {
    $feed_field->is_feed_item_field = ($is_feed_item_field === FALSE) ? '0' : '1';
    $feed_field->fid = $feed->fid;
    
    if (!$status = drupal_write_record('feed_field', $feed_field, isset($feed_field->ffid) ? 'ffid' : array())) {
      throw new Exception('Could not save a feed field record');
    }
    db_ignore_slave();
    
    return TRUE;
  }
  catch (Exception $e) {
    $transaction->rollback();
    watchdog_exception('feed', $e);
    return FALSE;
  }
}

/**
 * Delete feed fields.
 *
 * @param $ffids
 *   An array of feed field's ffid to delete.
 * @param $transaction (optional)
 *   A DatabaseTransaction object.
 *   
 * @return
 *   Status flag indicating outcome of the operation.
 */
function feed_fields_delete(array $ffids, DatabaseTransaction $transaction = NULL) {
  if (!$transaction) {
    $transaction = db_transaction();
  }

  try {
    foreach ($ffids as &$ffid) {
      if (is_object($ffid) && isset($ffid->ffid)) {
        $feed_field = $ffid;
      }
      else {
        $feed_field = feed_field_load($ffid);
      }
      
      if (isset($feed_field->ffid)) {
       if (!empty($feed_field->is_feed_item_field)) {
          $fifvids = db_select('feed_item_field_value', 'fifv')
            ->fields('fifv', array('fifvid'))
            ->condition('ffid', $feed_field->ffid)
            ->execute()->fetchAllKeyed(0, 0);
            
          if (!empty($fifvids) && !feed_item_field_values_delete($fifvids, $transaction)) {
            throw new Exception('Unable to delete feed item field values');
          }
        }
        else {
          $ffvids = db_select('feed_field_value', 'ffv')
            ->fields('ffv', array('ffvid'))
            ->condition('ffid', $feed_field->ffid)
            ->execute()->fetchAllKeyed(0, 0);
            
          if (!empty($ffvids) && !feed_field_values_delete($ffvids, $transaction)) {
            throw new Exception('Unable to delete feed field values');
          }
        }
      }
    }
    
    db_delete('feed_field')
      ->condition('ffid', $ffids)
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
 * Configure feed field object/ensure defaults
 * 
 * @param $feed_field
 * 	 A feed field object.
 */
function feed_field_defaults($feed_field = NULL) {
  $feed_field = (object) $feed_field;
  
  if (!isset($feed_field->name)) $feed_field->name = '';
  if (!isset($feed_field->tag)) $feed_field->tag = '';
  if (!isset($feed_field->namespace)) $feed_field->namespace = '';
  if (!isset($feed_field->module)) $feed_field->module = '';
  if (!isset($feed_field->is_required)) $feed_field->is_required = FALSE;
  
  if (!isset($feed_field->data)) {
    $feed_field->data = array(
      'is_editable' => FALSE,
      'instance' => array(),
      'weight' => 0,
      'title' => '',
      'description' => '',
    );
  }
  
  return $feed_field;
}

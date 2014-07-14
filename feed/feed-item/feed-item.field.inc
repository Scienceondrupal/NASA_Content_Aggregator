<?php 

/**
 * Theming function; Creates a feed's feed item field table.
 */
function theme_feed_item_field_attach($variables) {
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
  if (!empty($elements['item-fields'])) {
    foreach (element_children($elements['item-fields'], TRUE) as $name) {
      $field = $elements['item-fields'][$name];
      
      $indentation = array(
      	'#type' => 'markup',
      	'#markup' => theme('indentation', array('size' => $field['#depth'])),
      );
      
      $rows[] = array(
        'class' => array('draggable'),
        'data' => array(
          array('data' => array($indentation, $field['name'])),
          array('data' => array($field['weight'], $field['parent'], $field['id'])),
          array('data' => $field['tag']),
          array('data' => $field['type']),
          array('data' => $field['settings']),
        ),
      );
    }
  }
  
  // New field Rows.
  $row = array();
  $row[] = array('data' => $elements['add-item-field']['name']);
  $row[] = array('data' => array($elements['add-item-field']['weight'], $elements['add-item-field']['parent']));
  $row[] = array('data' => $elements['add-item-field']['tag']);
  $row[] = array('data' => $elements['add-item-field']['type']);
  $row[] = array('data' => '&nbsp;');
  
  $rows[] = array('data' => $row, 'class' => array('draggable', 'tabledrag-leaf'));
  
  drupal_add_tabledrag('feed-item-field', 'match', 'parent', 'item-field-parent', 'item-field-parent', 'item-field-id');
  drupal_add_tabledrag('feed-item-field', 'order', 'sibling', 'item-field-weight');
  
  return theme('feed_block', array(
    'attributes' => array('id' => 'feed-item-field-container'),
    'title' => 'Feed Item Fields',
    'content' => array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
      '#attributes' => array(
      	'id' => 'feed-item-field',
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
function feed_item_field_attach(&$form, &$form_state) {
  $feed = $form_state['#feed'];
  
  $form['feed_item_field'] = array();
  $form['feed_item_field']['#type'] = 'feed_item_field_attach';
  $form['feed_item_field']['#tree'] = TRUE;
  
  if (!empty($feed->fid)) {
    $feed_fields = feed_load_fields($feed, TRUE);
    feed_field_build_tree($feed_fields);
    
    foreach ($feed_fields as $feed_field) {
      if (!isset($form['feed_item_field']['item-fields'])) {
        $form['feed_item_field']['item-fields'] = array('#tree' => TRUE);
      }
      
      $form['feed_item_field']['item-fields'][$feed_field->ffid] = array(
        '#tree' => TRUE, 
        '#depth' => $feed_field->depth,
        '#weight' => isset($feed_field->data['weight']) ? $feed_field->data['weight'] : 0,
      );
      
      $form['feed_item_field']['item-fields'][$feed_field->ffid]['id'] = array(
        '#type' => 'hidden',
        '#value' => !empty($feed_field->ffid) ? $feed_field->ffid : NULL,
        '#attributes' => array(
          'class' => array('item-field-id'),
        ),
      );
      $form['feed_item_field']['item-fields'][$feed_field->ffid]['name'] = array(
        '#type' => 'markup',
        '#markup' => !empty($feed_field->name) ? $feed_field->name : 'No Label',
        '#attributes' => array('class' => array('item-field-name')),
        '#title_display' => 'invisible',
        '#title' => t('Label'),
      );
      $form['feed_item_field']['item-fields'][$feed_field->ffid]['weight'] = array(
        '#type' => 'weight',
        '#delta' => 50,
        '#default_value' => isset($feed_field->data['weight']) ? $feed_field->data['weight'] : 0,
        '#title_display' => 'invisible',
        '#title' => t('Weight for field'),
        '#attributes' => array('class' => array('item-field-weight')),
      );
      $form['feed_item_field']['item-fields'][$feed_field->ffid]['parent'] = array(
        '#type' => 'hidden',
        '#default_value' => !empty($feed_field->pffid) ? $feed_field->pffid : NULL,
        '#attributes' => array(
          'class' => array('item-field-parent'),
        ),
      );
      $form['feed_item_field']['item-fields'][$feed_field->ffid]['tag'] = array(
        '#type' => 'hidden',
        '#prefix' => !empty($feed_field->tag) ? $feed_field->tag : 'No Tag',
        '#value' => !empty($feed_field->tag) ? $feed_field->tag : '',
        '#attributes' => array('class' => array('item-field-machine-name')),
      );
      $form['feed_item_field']['item-fields'][$feed_field->ffid]['type'] = array(
        '#type' => 'markup',
        '#markup' => !empty($feed_field->type) ? $feed_field->type : '',
      );
      $form['feed_item_field']['item-fields'][$feed_field->ffid]['settings'] = array(
        '#type' => 'link',
        '#title' => t('Settings'),
        '#href' => "feed/{$feed->fid}/fields/{$feed_field->ffid}",
        '#options' => array('attributes' => array('title' => t('Edit feed field settings.'))),
      );
    }
  }
  
  // Add New field attribute.
  $form['feed_item_field']['add-item-field'] = array('#tree' => TRUE);
  $form['feed_item_field']['add-item-field']['name'] = array(
    '#type' => 'textfield',
    '#title' => t('New field label'),
    '#title_display' => 'invisible',
    '#size' => 15,
    '#description' => t('Feed item field label'),
  );
  $form['feed_item_field']['add-item-field']['weight'] = array(
    '#type' => 'weight',
    '#delta' => 50,
    '#default_value' => 0,
    '#title_display' => 'invisible',
    '#title' => t('Weight for new field'),
    '#description' => t('Weight'),
    '#attributes' => array(
      'class' => array('item-field-weight'),
    ),
  );
  $form['feed_item_field']['add-item-field']['parent'] = array(
    '#type' => 'hidden',
    '#default_value' => '',
    '#title_display' => 'invisible',
    '#attributes' => array(
      'class' => array('item-field-parent'),
    ),
  );
  $form['feed_item_field']['add-item-field']['tag'] = array(
    '#type' => 'textfield',
    '#title' => t('New field tag name'),
    '#title_display' => 'invisible',
    '#description' => t('Tag of this field'),
  );
  $form['feed_item_field']['add-item-field']['type'] = array(
    '#type' => 'select',
    '#title' => t('Type of new field'),
    '#title_display' => 'invisible',
    '#options' => feed_field_type_options(),
    '#empty_option' => t('- Select a type -'),
    '#description' => t('Type of field.'),
    '#attributes' => array('class' => array('item-field-type')),
  );
}

/**
 * Submit handler for feed_item_field_attach() elements.
 */
function feed_item_field_attach_submit($form, &$form_state) {
  $feed =& $form_state['#feed'];
  
  if (isset($feed->fid)) {
    $feed_fields = feed_load_fields($feed, TRUE);
    $feed_fields_stack = array();
    
    if (!empty($feed_fields)) {
      foreach ($feed_fields as $feed_field) {
        if (isset($form_state['values']['feed_item_field']['item-fields'][$feed_field->ffid])) {
          $values = $form_state['values']['feed_item_field']['item-fields'][$feed_field->ffid];
          
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
  
  if (!empty($form_state['values']['feed_item_field']['add-item-field']['tag'])) {
    $feed_field_values = $form_state['values']['feed_item_field']['add-item-field'];
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
    
    $feed_fields_stack['add-item-field'] = $feed_field;
  }
  
  if (!empty($feed_fields_stack)) {
    $status = feed_fields_save($feed_fields_stack, $feed, TRUE);
    
    if ($status && !empty($feed_fields_stack['add-item-field'])) {
      $form_state['redirect'] = "feed/{$feed->fid}/fields/feed-items/{$ffid}";
    }
  }
}

/**
 * Create feed item field settings elements
 */
function feed_item_field_settings_attach(&$form, &$form_state) {
  $feed = $form_state['#feed'];
  $feed_field = $form_state['#feed_field'];

  if ($info = feed_field_type_info($feed_field->type, TRUE)) {
    if (isset($info['form_settings'])) {
      $settings = !empty($info['settings']) ? $info['settings'] : array();
      $instance = array_merge($settings, feed_object_get_data($feed_field, 'instance', array()));
      
      $function = (string) $info['form_settings'];
      
      if (function_exists($function)) {
        $elements = $function($feed_field, $instance);
      
        if (is_array($elements) && !empty($elements)) {
          if (!isset($form['feed_item_field_settings'])) {
            $form['feed_item_field_settings'] = array(
            	'#tree' => TRUE,
              '#type' => 'feed_fieldset',
              '#title' => t('Feed Item Field Settings'),
            );
          }
          
          if (!isset($form['feed_item_field_settings'][$feed_field->fid])) {
            $form['feed_item_field_settings'][$feed_field->fid] = array();
          }
          $form['feed_item_field_settings'][$feed_field->fid] += $elements;
        }
      }
    }
  }
}

/**
 * Submit handler for feed_item_field_settings_attach() elements.
 */
function feed_item_field_settings_attach_submit(&$form, &$form_state) {
  $feed_field = $form_state['#feed_field'];
  
  if (!empty($form_state['values']['feed_item_field_settings'])) {
    feed_item_field_settings_apply($feed_field, $form_state['values']['feed_item_field_settings'][$feed_field->fid]);
  }
}

/**
 * Apply feed item field setting values to the feed field.
 * 
 * @param $feed_field
 *   A feed field object.
 * @param $values
 *   Array of values to apply.
 */
function feed_item_field_settings_apply(&$feed_field, $values = NULL) {
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
    $feed_field_value->data['instance'] = $data;
  }
}

/**
 * Form process element for a feed item field widget
 */
function form_process_feed_item_field_widget(&$element, &$form_state) {
  if (!$info = feed_field_type_info($element['#feed_item_field']->type, TRUE)) {
    return FALSE;
  }
  $feed_item_field = &$element['#feed_item_field'];
  $feed_item_field_values = &$element['#feed_item_field_values'];
  
  $element['widget'] = array();
  $element['widget']['#type'] = 'container';
  
  if (empty($feed_item_field_values)) {
    $feed_item_field_values = array('new' => feed_item_field_value_defaults());
  }
  
  if (isset($info['form_widget'])) {
    $settings = !empty($info['settings']) ? $info['settings'] : array();
    $instance = array_merge($settings, feed_object_get_data($feed_item_field, 'instance', array()));
    
    $function = (string) $info['form_widget'];
    
    if (!empty($feed_item_field->is_required)) {
      $instance['required'] = TRUE;
    }
    
    foreach ($feed_item_field_values as $fifvid => $feed_item_field_value) {
      $values = feed_object_get_data($feed_item_field_value, FEED_DATA_TYPE_FIELD_VALUE, array());
      $element['widget'][$fifvid] = $function($feed_item_field, $instance, $values);
    }
  }
  return $element;
}

/**
 * Form validator handler to save feed field widget.
 */
function form_validate_feed_item_field_widget(&$element, &$form_state) {
  $feed_item_field = &$element['#feed_item_field'];
  $feed_item_field_values = &$element['#feed_item_field_values'];

  $values = drupal_array_get_nested_value($form_state['values'], $element['#parents']);
  
  if (!empty($values)) {
    feed_item_field_values_apply($feed_item_field, $feed_item_field_values, $values['widget']);
  }
}

/**
 * Load a feed item's field by tag name & namespace
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
function feed_item_field_tagname_load($tagname, $fid = NULL, $reload = FALSE) {
  $feed_item_fields = &drupal_static(__FUNCTION__, array());
  
  if (!isset($feed_item_fields[$tagname])) {
    $feed_item_fields[$tagname] = array();
  }
  if ($reload || empty($feed_item_fields[$tagname])) {
    if (strpos($tagname, ':') === FALSE) {
      $tag = $tagname;
    }
    else {
      list($tag, $namespace) = explode(':', $tagname, 2);
    }
    
    $query = db_select('feed_field', 'ff')
      ->fields('ff', array('ffid'))
      ->condition('ff.is_feed_item_field', TRUE)
      ->condition('ff.tag', $tag);
      
    if (!empty($namespace)) {
      $query->condition('ff.namespace', $namespace);
    }
    $feed_item_fields[$tagname] = $query->execute()->fetchAllKeyed(0, 0);
  }
  
  if (!empty($fid)) {
    $result = array();
    if ($feed = feed_load($fid, $reload)) {
      foreach (feed_load_fields($feed, TRUE, $reload) as $field) {
        if (in_array($field->ffid, $feed_item_fields[$tagname])) {
          $result[$field->ffid] = $field;
        }
      }
    }
    return $result;
  }
  return feed_fields_load($feed_item_fields[$tagname], $reload);
}
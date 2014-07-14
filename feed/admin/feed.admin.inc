<?php

/**
 * Menu callback; Shows the feed list for all content.
 */
function feed_all_overview() {
  drupal_set_title('All Feeds');
  
  $feeds = feed_load_all(array(FEED_TYPE_CREATED, FEED_TYPE_IMPORTED));

  $header = array(
    array('data' => t('Title'), 'class' => array('feed-col-1')),
    array('data' => t('Description'), 'class' => array('feed-col-2')), 
    array('data' => t('Source'), 'class' => array('feed-col-3')), 
    array('data' => t('Operations'), 'colspan' => '3', 'class' => array('feed-col-4')),
  );
  $rows = array();

  foreach ($feeds as $feed) {
    $group = feed_group_user_get_primary($feed->uid);
    $title = feed_field_values_tagname_format($feed->fid, 'title', 'text');
    $description = feed_field_values_tagname_format($feed->fid, 'description', 'text');
    
    $row = array();
    $row[] = array('data' => l($title, "feed/{$feed->fid}/format/web"), 'class' => array('feed-col-1'));
    $row[] = array('data' => $description, 'class' => array('feed-col-2'));
    $row[] = array('data' => !empty($group->name) ? $group->name : 'None', 'class' => array('feed-col-3'));
    $row[] = array('data' => l(t('RSS'), "feed/{$feed->fid}/format/rss"), 'class' => array('feed-col-4'));
    $row[] = array('data' => l(t('subscribe'), "feed/all/subscription/{$feed->fid}"), 'class' => array('feed-col-5'));
    $row[] = array('data' => l(t('widget'), "feed/{$feed->fid}/widget"), 'class' => array('feed-col-6'));
    
    $rows[] = $row;
  }
 
  $build = array();
  $build['feed_all_overview'] = array(
    '#theme' => 'feed_block',
    '#attributes' => array('id' => 'feed-all-overview'),
    '#title' => 'List of All Feeds',
    '#content' => array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
      '#empty' => t('No feeds available.'),
    ),
  );
  return $build;
}

/**
 * Menu callback; Shows the feed list.
 */
function feed_overview() {
  $feeds = feed_load_all(array(FEED_TYPE_CREATED, FEED_TYPE_IMPORTED));

  $header = array(
    array('data' => t('Title'), 'class' => array('feed-col-1')),
    array('data' => t('Manage Feed'), 'colspan' => '4', 'class' => array('feed-col-2')),
    array('data' => t('Manage Items'), 'colspan' => '2', 'class' => array('feed-col-3')),
  );
  
  $rows = array();

  foreach ($feeds as $feed) {
    if (feed_access($feed)) {
      $title = feed_field_values_tagname_format($feed->fid, 'title', 'text');
      
      $row = array();
      $row[] = array('data' => l($title, "feed/{$feed->fid}/format/web", array('html' => TRUE)), 'class' => array('feed-col-1'));
      $row[] = array('data' => l(t('RSS'), "feed/{$feed->fid}/format/rss"), 'class' => array('feed-col-2'));
      $row[] = array('data' => l(t('Widget'), "feed/{$feed->fid}/widget"), 'class' => array('feed-col-3'));
      $row[] = array('data' => l(t('edit feed'), "feed/{$feed->fid}/edit"), 'class' => array('feed-col-4'));
      $row[] = array('data' => l(t('delete feed'), "feed/{$feed->fid}/delete"), 'class' => array('feed-col-5'));
      $row[] = array('data' => l(t('view items'), "feed/{$feed->fid}/feed-items"), 'class' => array('feed-col-6'));
      $row[] = array('data' => l(t('add item'), "feed/{$feed->fid}/feed-items/add"), 'class' => array('feed-col-7'));
  
      $rows[] = $row;
    }
  }
  $build = array();
  $build['feed_links'] = array(
    '#theme' => 'links',
    '#attributes' => array('class' => array('feed-links')),
    '#links' => array(
      'add_feed' => array(
        'title' => 'Add Feed',
        'href' => 'feed/add',
        'attributes' => array(
          'class' => array('button'),
        ),
      ),
      'import_feed' => array(
        'title' => 'Import Feed From URL',
        'href' => 'feed/import',
        'attributes' => array(
          'class' => array('button'),
        ),
      ),
      'categories' => array(
        'title' => 'Categories',
        'href' => 'feed/categories',
        'attributes' => array(
          'class' => array('button'),
        ),
      ),
    ),
  );
  $build['feed_overview'] = array(
    '#theme' => 'feed_block',
    '#attributes' => array('id' => 'feed-overview'),
    '#title' => 'Feeds List',
    '#content' => array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
      '#empty' => t('No feeds available. <a href="@link">Add feed</a>.', array('@link' => url('feed/add'))),
    ),
  );
  return $build;
}

/**
 * Feed import form; Imports a feed from a http url.
 */
function feed_import_form($form, &$form_state) {
  if (!isset($form_state['#feed_build_state'])) {
    $form_state['#feed_build_state'] = 'create';
  }
  
  switch ($form_state['#feed_build_state']) {
    case 'configure':
      feed_import_config($form, $form_state);
    break;
    
    case 'create':
    default:
      feed_import_create($form, $form_state);
    break;
  }
  return $form;
}

/**
 * Feed import form submit handler.
 */
function feed_import_form_submit(&$form, &$form_state) {
  switch ($form_state['#feed_build_state']) {
    case 'configure':
      feed_import_config_submit($form, $form_state);
    break;
    
    case 'create':
    default:
      feed_import_create_submit($form, $form_state);
    break;
  }
}

/**
 * Feed import configuration form;
 */
function feed_import_config(&$form, &$form_state) {
  if (!empty($form_state['#feed_item_field_configs'])) {
    $form['feed_item_field'] = array();
    $form['feed_item_field']['#type'] = 'feed_table';
    $form['feed_item_field']['#tree'] = TRUE;
    $form['feed_item_field']['#header'] = array(t('Label'), t('Tag'), t('Type'));
  
    foreach ($form_state['#feed_item_field_configs'] as $feed_item_index => $configs) {
      foreach ($configs as $identifier => $config) {
        if (isset($form['feed_item_field'][$identifier]) || empty($config['configurable'])) {
          continue;
        }
        
        $form['feed_item_field'][$identifier] = array('#tree' => TRUE);
        $form['feed_item_field'][$identifier]['name'] = array(
          '#type' => 'textfield',
          '#default_value' => !empty($config['name']) ? $config['name'] : '',
          '#title_display' => 'invisible',
          '#title' => t('Label'),
        	'#description' => t('Administrative label.'),
          '#attributes' => array('class' => array('field-name')),
        );
        
        $form['feed_item_field'][$identifier]['tag'] = array(
          '#type' => 'textfield',
          '#title' => t('Name'),
          '#title_display' => 'invisible',
          '#value' => !empty($config['tag']) ? $config['tag'] : '',
          '#description' => t('XML tag name.'),
          '#attributes' => array('class' => array('field-tag')),
          '#disabled' => TRUE,
        );
        $form['feed_item_field'][$identifier]['structure'] = array(
          '#type' => 'select',
          '#title' => t('Type of new field'),
          '#title_display' => 'invisible',
          '#options' => $config['types'],
          '#default_value' => isset($config['default']) ? $config['default'] : reset(array_keys($config['types'])),
          '#empty_option' => t('- Select a type -'),
          '#description' => t('Type of field.'),
          '#attributes' => array('class' => array('field-type')),
          '#disabled' => !empty($config['types']) ? FALSE : TRUE,
        );
      }
    }
  }
  
  $form['#attributes'] = array('class' => array('feed-form'));
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Import Feed Items'),
    '#weight' => 40,
  );
}

/**
 * Feed import configuration form submit handler;
 */
function feed_import_config_submit(&$form, &$form_state) {
  $feed = &$form_state['#feed'];
  $feed_fields = default_feed_fields();
  $feed_items = array();
  $feed_item_fields = default_feed_item_fields();
  $feed_elements = &$form_state['#elements'];
  $feed_configs = &$form_state['#feed_configs'];
  
  foreach ($feed_configs['feed_item_field_configs'] as $feed_item_index => &$configs) {
    foreach ($configs as $field_identifier => &$config) {
      if (isset($form_state['values']['feed_item_field'][$field_identifier])) {
        $values = $form_state['values']['feed_item_field'][$field_identifier];
        
        if (!empty($values['structure']) && array_key_exists($values['structure'], $config['structures'])) {
          $config['structure'] = $values['structure'];
        }
        
        if (!empty($values['name'])) {
          if (!isset($feed_item_fields[$field_identifier])) {
            $feed_item_fields[$field_identifier] = feed_field_defaults();
          }
          $feed_item_fields[$field_identifier]->name = check_plain($values['name']);
        }
      }
    }
  }
  
  if (!feed_import_save($feed, $feed_fields, $feed_items, $feed_item_fields, $feed_elements, $feed_configs)) {
    form_set_error('feed_item_field', 'There was an error building the feed, please try again.');
  }
  else {
    $form_state['redirect'] = 'feed';
  }
}

/**
 * Feed import create form;
 */
function feed_import_create(&$form, &$form_state) {
  $form_state['#feed'] = feed_defaults();
  $parsers = feed_parser_info_options();
  
  $form['url'] = array(
  	'#type' => 'textfield',
    '#title' => t('URL'),
    '#default_value' => '',
    '#maxlength' => 255,
    '#description' => t('The fully-qualified URL of the feed.'),
    '#required' => TRUE,
  );
  $form['parser'] = array(
    '#type' => 'select',
    '#title' => t('Parser'),
    '#default_value' => '',
    '#options' => $parsers,
    '#description' => t('The type of feed this URL location should be parsed as.'),
    '#required' => TRUE,
  );
  
  $form_state['#feed_refresh_times'] = array(
    'hourly' => 3600, 
    'daily' => 86400, 
    'weekly'=> 604800, 
    'monthly' => 2592000,
  );
  
  $form['refresh'] = array(
  	'#type' => 'select',
    '#title' => t('Update Interval'),
    '#default_value' => 'weekly',
    '#options' => drupal_map_assoc(array_keys($form_state['#feed_refresh_times'])),
    '#description' => t('The length of time between feed updates.'),
  );
  $form['#attributes'] = array('class' => array('feed-form'));
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['import'] = array(
    '#type' => 'submit',
    '#value' => t('Import Feed'),
    '#weight' => 40,
  );
  $form['actions']['configure'] = array(
    '#attributes' => array('style' => 'display: none;'),
    '#type' => 'submit',
    '#value' => t('Configure Feed Item Fields'),
    '#weight' => 40,
  );
}

/**
 * Feed import create form submit handler;
 */
function feed_import_create_submit(&$form, &$form_state) {
  $refresh = 604800;
  $values = $form_state['values'];
  
  if (valid_url($values['url'])) {
    $form_state['#feed']->type = FEED_TYPE_IMPORTED;
    
    if ($response = feed_fetch($form_state['#feed'], $values['url'])) {
      try {
        if ($parser = create_xml_parser($response->data)) {
          $elements = $parser->parse();
          
          $form_state['#feed_configs'] = feed_config_parse($values['parser'], $elements);
          
          if (isset($form_state['#feed_refresh_times'][$values['refresh']])) {
            $refresh = $form_state['#feed_refresh_times'][$values['refresh']];
          }
          
          feed_object_set_data($form_state['#feed'], FEED_DATA_FEED_UPDATE_URL, $values['url']);
          feed_object_set_data($form_state['#feed'], FEED_DATA_FEED_PARSER, $values['parser']);
          feed_object_set_data($form_state['#feed'], FEED_DATA_FEED_UPDATE_FREQ, $refresh);
          feed_object_set_data($form_state['#feed'], FEED_DATA_FEED_UPDATE_QUEUED, FALSE);
          feed_object_set_data($form_state['#feed'], FEED_DATA_FEED_UPDATE_REFRESH, REQUEST_TIME + $refresh);
          
          $form_state['#elements'] = $elements;
        }
      }
      catch (Exception $e) {
        watchdog_exception('feed', $e);
        form_set_error('url', 'Error processing data from URL. Check URL is a valid xml feed (i.e. http://earthdata.nasa.gov/rss.xml).');
      }
    }
    else {
      form_set_error('url', 'No response from URL. Verify the URL is valid.');
    }
  }
  
  if ($form_state['values']['op'] == $form['actions']['import']['#value']) {
    $feed = &$form_state['#feed'];
    $feed_fields = default_feed_fields();
    $feed_items = array();
    $feed_item_fields = default_feed_item_fields();
    $feed_elements = &$form_state['#elements'];
    $feed_configs = &$form_state['#feed_configs'];
    
    if (!feed_import_save($feed, $feed_fields, $feed_items, $feed_item_fields, $feed_elements, $feed_configs)) {
      form_set_error('feed_item_field', 'There was an error building the feed, please try again.');
    }
    else {
      $form_state['redirect'] = 'feed';
    }
  }
  elseif ($form_state['values']['op'] == $form['actions']['configure']['#value']) {
    $form_state['#feed_build_state'] = 'configure';
    $form_state['rebuild'] = TRUE;
  }
}

/**
 * Helper function for feed_import_form; Saves an imported feed.
 */
function feed_import_save($feed, $feed_fields, $feed_items, $feed_item_fields, $feed_elements, $feed_configs) {
  $parser = feed_object_get_data($feed, FEED_DATA_FEED_PARSER);
  if (empty($feed_elements) || empty($parser) || empty($feed)) {
    return FALSE;
  }
  
  $build = array(
    'feed' => &$feed,
    'feed_fields' => &$feed_fields,
    'feed_items' => &$feed_items,
    'feed_item_fields' => &$feed_item_fields,
  );
  $objects = feed_parse($build, $parser, $feed_elements, $feed_configs);
  
  feed_build_save(array(
    'feed' => $objects['feed'],
    'feed_items' => $objects['feed_items'],
    'feed_fields' => $objects['feed_fields'],
    'feed_field_values' => $objects['feed_field_values'],
    'feed_item_fields' => $objects['feed_item_fields'],
    'feed_item_field_values' => $objects['feed_item_field_values'],
  ));
  feed_item_publish($objects['feed_items']);
  
  return TRUE;
}

/**
 * Manage feed form;
 */
function feed_form($form, &$form_state, $feed = NULL) {
  $form_state['#feed'] = feed_defaults($feed);
  $form_state['#feed_fields'] = !empty($feed->fid) ? feed_load_fields($feed, FALSE) : default_feed_fields();
  
  $form['feed'] = array(
    '#type' => 'feed_fieldset',
    '#title' => 'Create RSS Feed',
    '#tree' => TRUE,
    '#attributes' => array(
      'class' => array('feed-create-container'),
    )
  );
  
  // Run handlers to attach feed widgets
  $form['feed']['feed_fields_required'] = array('#type' => 'container');
  $form['feed']['feed_fields'] = array('#type' => 'container');
  
  foreach ($form_state['#feed_fields'] as $ffid => &$feed_field) {
    $key = !empty($feed_field->is_required) ? 'feed_fields_required' : 'feed_fields';
    
    $feed_field_values = !empty($form_state['#feed']->fid) ? feed_field_values_load($form_state['#feed'], $feed_field) : array();
    $form['feed'][$key][$ffid] = array(
      '#type' => 'feed_field_widget',
      '#feed_field' => &$feed_field,
      '#feed_field_values' => $feed_field_values,
    );
  }
  
  $form['#attributes'] = array('class' => array('feed-form'));
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save feed'),
    '#weight' => 40,
    '#attributes' => array('class' => array('feed-submit')),
  );
  return $form;
}

/**
 * Manage feed form submit handler;
 */
function feed_form_submit($form, &$form_state) {
  $feed =& $form_state['#feed'];
  $feed->type = FEED_TYPE_CREATED;
  $is_new = empty($feed->fid);
  
  if ($status = feed_save($feed)) {
    if ($is_new) {
      foreach (default_feed_item_fields() as $feed_item_field) {
        feed_field_save($feed_item_field, $feed, TRUE);
      }
    }
    
    foreach (array('feed_fields_required', 'feed_fields') as $widget_key) {
      foreach (element_children($form['feed'][$widget_key]) as $key) {
        $feed_field_widget  = $form['feed'][$widget_key][$key];
        
        if (isset($feed_field_widget['#feed_field_values'], $feed_field_widget['#feed_field'])) {
          if (!isset($feed_field_widget['#feed_field']->ffid) && !feed_field_save($feed_field_widget['#feed_field'], $feed, FALSE)) {
            drupal_set_message(t('Error saving value for field %field.', array('%field' => $feed_field_widget['#feed_field']->name)), 'error');
          }
          
          if (isset($feed_field_widget['#feed_field']->ffid) && !feed_field_values_save($feed_field_widget['#feed_field_values'], $feed, $feed_field_widget['#feed_field'])) {
            drupal_set_message(t('Error saving value for field %field.', array('%field' => $feed_field_widget['#feed_field']->name)), 'error');
          }
        }
      }
    }
    $title = !empty($feed->fid) ? feed_field_values_tagname_format($feed->fid, 'title', 'text') : '';
    
    if ($status == SAVED_UPDATED) {
      drupal_set_message(t('The feed "%title" has been updated.', array('%title' => $title)));
    }
    elseif ($status == SAVED_NEW) {
      drupal_set_message(t('The feed "%title" has been added.', array('%title' => $title)));
    }
    $form_state['redirect'] = 'feed';
  }
}

/**
 * Delete feed form;
 */
function feed_delete_form($form, &$form_state, $feed) {
  $form_state['#feed'] = feed_defaults($feed);
  $form['#attributes'] = array('class' => array('feed-form'));
  
  $title = feed_field_values_tagname_format($feed->fid, 'title', 'text');
  return confirm_form(
    $form,
    t('Delete feed?'),
    'feed',
    '<div class="messages warning"><span class="feed-warning">Warning:</span> ' . 
    t('You are about to delete feed ":title". This action cannot be undone and all feed data will be lost', array(':title' => $title)) . '</div>',
    t('Delete'),  t('Cancel')
  );
}

/**
 * Delete feed form submit handler;
 */
function feed_delete_form_submit(&$form, &$form_state) {
  $feed = $form_state['#feed'];
  
  $title = feed_field_values_tagname_format($feed->fid, 'title', 'text');
  if (feed_delete($feed)) {
    drupal_set_message(t('Feed ":title" was deleted.', array(':title' => $title)));
  } 
  else {
    drupal_set_message(t('Could not delete feed ":title".', array(':title' => $title)), 'error');
  }
  $form_state['redirect'] = 'feed';
}

/**
 * Feed field settings form;
 */
function feed_field_settings_form($form, &$form_state, $feed = NULL, $feed_field = NULL) {
  if ($feed_field->is_feed_item_field) {
    drupal_goto("feed/{$feed->fid}/fields/feed-items/{$feed_field->ffid}"); exit();
  }
  
  $form_state['#feed'] = feed_defaults($feed);
  $form_state['#feed_field'] = feed_field_defaults($feed_field);
  
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Label'),
    '#default_value' => $form_state['#feed_field']->name,
    '#description' => t('The administrative label for this feed field'),
    '#required' => TRUE,
  );
  
  $form['tag'] = array(
    '#type' => 'textfield',
    '#title' => t('Tag Name'),
    '#default_value' => $form_state['#feed_field']->tag,
    '#description' => t('The tag name for this feed field'),
    '#required' => TRUE,
  );
  
  $form['formatters'] = array(
    '#tree' => TRUE,
    '#type' => 'feed_fieldset',
    '#title' => t('Feed Field Display'),
  );
  
  $display_settings = feed_object_get_data($feed_field, FEED_DATA_FEED_FIELD_DISPLAY_SETTINGS, array());
  $display_enabled = feed_object_get_data($feed_field, FEED_DATA_FEED_FIELD_DISPLAY_ENABLED, array());
  
  $formatters = feed_formatter_info();
  
  foreach ($formatters as $formatter => $info) {
    $display_setting = array();
    if (isset($display_settings[$formatter])) {
      $display_setting += $display_settings[$formatter];
    }
    if (isset($info['feed_field_settings_defaults'])) {
      $display_setting += $display_settings[$formatter];
    }
    $form['formatters']['display_' . $formatter] = array(
      '#type' => 'checkbox',
      '#default_value' => isset($display_enabled[$formatter]) ? !empty($display_enabled[$formatter]) : TRUE,
      '#title' => t('Display in :formatter format', array(':formatter' => $formatter)),
      '#description' => t('Displays this field in :formatter output format.', array(':formatter' => $formatter)),
    );
    if ($element = feed_formatter_invoke($formatter, 'feed_field_setting', $feed_field, $display_setting)) {
      $form['formatters'][$formatter]['#tree'] = TRUE;
      $form['formatters'][$formatter]['#type'] = 'feed_fieldset';
      $form['formatters'][$formatter]['#title'] = t('Settings for :formatter formatter', array(':formatter' => $formatter));
      $form['formatters'][$formatter]['#states'] = array(
        'visible' => array('#edit-formatters-display-' . $formatter => array('checked' => TRUE)),
      );
      $form['formatters'][$formatter] += $element;
    }
  }
  
  // Run handlers to attach feed field settings
  feed_field_settings_attach($form, $form_state);
  
  $form['#attributes'] = array('class' => array('feed-form'));
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save feed field settings'),
    '#weight' => 40,
  );
  
  return $form;
}

/**
 * Feed field settings form submit handler;
 */
function feed_field_settings_form_submit($form, &$form_state) {
  $feed =& $form_state['#feed'];
  
  $feed_field =& $form_state['#feed_field'];
  $feed_field->name = trim($form_state['values']['name']);
  $feed_field->tag = trim($form_state['values']['tag']);
  
  $display_settings = feed_object_get_data($feed_field, FEED_DATA_FEED_FIELD_DISPLAY_SETTINGS, array());
  $display_enabled = feed_object_get_data($feed_field, FEED_DATA_FEED_FIELD_DISPLAY_ENABLED, array());
  
  if (!empty($form_state['values']['formatters'])) {
    $formatters = feed_formatter_info();
    
    foreach ($formatters as $formatter => $info) {
      $display_enabled[$formatter] = !empty($form_state['values']['formatters']['display_' . $formatter]);
      
      if (!empty($form_state['values']['formatters'][$formatter])) {
        $display_settings[$formatter] = $form_state['values']['formatters'][$formatter];
      }
    }
    feed_object_set_data($feed_field, FEED_DATA_FEED_FIELD_DISPLAY_ENABLED, $display_enabled);
    feed_object_set_data($feed_field, FEED_DATA_FEED_FIELD_DISPLAY_SETTINGS, $display_settings);
  }
  
  // Run handlers to apply feed field settings
  feed_field_settings_attach_submit($form, $form_state);

  $feed_fields = array();
  $feed_fields[] =& $feed_field;
  
  $status = feed_fields_save($feed_fields, $feed, FALSE);
  
  if ($status !== FALSE) {
    if ($status == SAVED_UPDATED) {
      drupal_set_message(t('The feed field %name has been updated.', array('%name' => $feed_field->name)));
    }
    elseif ($status == SAVED_NEW) {
      drupal_set_message(t('The feed field %name has been added.', array('%name' => $feed_field->name)));
    }
  }
}
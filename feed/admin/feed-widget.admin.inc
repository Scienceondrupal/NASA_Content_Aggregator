<?php

/**
 * Menu callback; Shows the feed widget list.
 */
function feed_widget_overview($feed) {
  $feed_widgets = feed_widget_load_all_feed($feed->fid);
  
  $header = array(
    array('data' => t('Name'), 'class' => array('feed-col-1')),
    array('data' => t('Operations'), 'class' => array('column-2'), 'colspan' => '3'),
  );
  $rows = array();

  foreach ($feed_widgets as $feed_widget) {
    $rows[] = array(
      array('data' => t($feed_widget->name)),
      array('data' => '<a href="#" class="feed-widget-show-snippet" data-widget-target="feed-widget-snippet-' . $feed_widget->fwid . '">show html snippet</a>'),
      array('data' => l(t('edit'), "feed/{$feed->fid}/widget/{$feed_widget->fwid}/edit")),
      array('data' => l(t('delete'), "feed/{$feed->fid}/widget/{$feed_widget->fwid}/delete")),
    );
    
    $rows[] = array(
      'data' => array(
        array(
        	'data' => "<pre><xmp><div id=\"eosdis-feed-widget\"></div><script type=\"text/javascript\" \nsrc=\"" . file_create_url($feed_widget->file) . "\"></script></xmp></pre>",
        	'colspan' => '5',
          'class' => 'feed-widget-code',
        ),
      ),
      'class' => array('feed-widget-code-row', 'feed-widget-snippet-' . $feed_widget->fwid),
    );
  }  
  
  $build = array();
  $build['feed_links'] = array(
    '#theme' => 'links',
    '#attributes' => array('class' => array('feed-links')),
    '#links' => array(
      'add_feed' => array(
        'title' => 'Add Widget',
        'href' => "feed/{$feed->fid}/widget/add",
        'attributes' => array(
          'class' => array('button'),
        ),
      ),
    ),
  );
  $build['feed_widget_overview'] = array(
    '#theme' => 'feed_block',
    '#attributes' => array('id' => 'feed-widget-overview'),
    '#title' => 'Feed Widget List',
    '#content' => array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
      '#attributes' => array('class' => array('feed-widget-overview-table')),
    	'#empty' => t('No feed widgets available. <a href="@link">Add feed widget</a>.', array('@link' => url("feed/{$feed->fid}/widget/add"))),
    ),
  );
  return $build;
}

/**
 * Feed widget form; Creates the javascript snippet for a feed widget.
 */
function feed_widget_form($form, &$form_state, $feed, $feed_widget = NULL) {
  $form_state['#feed'] = $feed;
  $form_state['#feed_fields'] = feed_load_fields($feed, FALSE);
  $form_state['#feed_item_fields'] = $feed->type == FEED_TYPE_SUBSCRIBED ? default_feed_item_fields() : feed_load_fields($feed, TRUE);
  $form_state['#feed_widget'] = feed_widget_defaults($feed_widget);
  
  $config = &$form_state['#configuration'];
  
  if (!empty($feed_widget->configuration)) {
    $config = $feed_widget->configuration;
  }
  
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#description' => t('Name this widget for administrative reference.'),
    '#default_value' => $form_state['#feed_widget']->name,
  );
  $form['widget'] = array(
  	'#type' => 'container',
    '#tree' => TRUE,
    '#attributes' => array(
      'class' => array('feed-widget-form-option'),
    ),
  );
  
  $form['widget']['feed_fields'] = array(
    '#type' => 'feed_table',
    '#tree' => TRUE,
    '#title' => t('Feed Fields'),
    '#header' => array(
      t('Display Field'),
      t('Weight'),
      t('Format'),
    ),
    '#attributes' => array(
      'id' => 'feed-widget-field-table',
    ),
  );
  
  $displayed = !empty($config['show']['infos']) ? $config['show']['infos'] : array('title', 'link', 'description');
  $formatted = !empty($config['format']['infos']) ? $config['format']['infos'] : array(
    'title' => 'text',
    'link' => 'text',
    'description' => 'text',
  );
  
  foreach ($displayed as $name) {
    $form['widget']['feed_fields'][$name] = FALSE;
  }
  
  foreach ($form_state['#feed_fields'] as $feed_field) {
    $name = $feed_field->tag;
    if ($feed_field->namespace != XmlElement::DEFAULT_NAMESPACE) {
      $name = $feed_field->namespace . ':' . $name;
    }
    
    $form['widget']['feed_fields'][$name] = array('#tree' => TRUE, '#row' => array('class' => array('draggable')));
    $form['widget']['feed_fields'][$name]['display'] = array(
    	'#type' => 'checkbox',
      '#title' => ucwords($feed_field->name),
      '#default_value' => in_array($name, $displayed),
    );
    $form['widget']['feed_fields'][$name]['weight'] = array(
      '#type' => 'weight',
      '#attributes' => array(
        'class' => array('feed-widget-field-weight'),
      ),
    );
    $form['widget']['feed_fields'][$name]['format'] = array(
    	'#type' => 'select',
      '#title' => t('Format'),
      '#title_display' => 'invisible',
      '#default_value' => !empty($formatted[$name]) ? $formatted[$name] : 'text',
      '#options' => array(
      	'text' => t('Text'),
      	'web' => t('HTML'),
      ),
    );
  }
  
  drupal_add_tabledrag('feed-widget-field-table', 'order', 'sibling', 'feed-widget-field-weight');
  
  $form['widget']['feed_item_fields'] = array(
    '#type' => 'feed_table',
    '#tree' => TRUE,
    '#title' => t('Feed Item Fields'),
    '#header' => array(
      t('Display Field'),
      t('Weight'),
      t('Format'),
    ),
    '#attributes' => array(
      'id' => 'feed-widget-item-field-table',
    ),
  );
  
  $displayed = !empty($config['show']['items']) ? $config['show']['items'] : array('title', 'link', 'description');
  $formatted = !empty($config['format']['items']) ? $config['format']['items'] : array(
    'title' => 'text',
    'link' => 'text',
    'description' => 'text',
  );
  
  foreach ($displayed as $name) {
    $form['widget']['feed_item_fields'][$name] = FALSE;
  }
  
  $sorting_field_options = array();
  foreach ($form_state['#feed_item_fields'] as $feed_item_field) {
    $name = $feed_item_field->tag;
    if ($feed_item_field->namespace != XmlElement::DEFAULT_NAMESPACE) {
      $name = $feed_item_field->namespace . ':' . $name;
    }
    
    $sorting_field_options[$name] = ucwords($feed_item_field->name);
    
    $form['widget']['feed_item_fields'][$name] = array('#tree' => TRUE, '#row' => array('class' => array('draggable')));
    $form['widget']['feed_item_fields'][$name]['display'] = array(
    	'#type' => 'checkbox',
      '#title' => ucwords($feed_item_field->name),
      '#default_value' => in_array($name, $displayed),
    );
    $form['widget']['feed_item_fields'][$name]['weight'] = array(
      '#type' => 'weight',
      '#attributes' => array(
        'class' => array('feed-widget-item-field-weight'),
      ),
    );
    $form['widget']['feed_item_fields'][$name]['format'] = array(
    	'#type' => 'select',
      '#title' => ucfirst($feed_item_field->name) . ' Format',
      '#title_display' => 'invisible',
      '#default_value' => !empty($formatted[$name]) ? $formatted[$name] : 'text',
      '#options' => array(
      	'text' => t('Text'),
      	'web' => t('HTML'),
      ),
    );
  }
  
  drupal_add_tabledrag('feed-widget-item-field-table', 'order', 'sibling', 'feed-widget-item-field-weight');
  
  $form['widget']['limit'] = array(
    '#type' => 'textfield',
    '#title' => t('Items per page'),
    '#default_value' => !empty($config['limit']) ? $config['limit'] : '5',
    '#size' => 20,
  );
  $form['widget']['random'] = array(
    '#type' => 'checkbox',
    '#title' => t('Randomize'),
    '#attributes' => array('class' => array('feed-widget-option-random')),
    '#default_value' => !empty($config['random']),
  );
  $form['widget']['allow_pagination'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow Pagination'),
    '#default_value' => !empty($config['paginate']) ? $config['paginate'] : FALSE,
    '#states' => array(
      'invisible' => array('.feed-widget-option-random' => array('checked' => TRUE)),
    ),
  );
  $form['widget']['allow_sorting'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow Sorting'),
    '#default_value' => !empty($config['sorting']) ? $config['sorting'] : FALSE,
  );
  $form['widget']['sorting_field'] = array(
    '#type' => 'select',
    '#title' => t('Sort By'),
    '#default_value' => !empty($config['sort']) ? $config['sort'] : 'publish',
    '#options' => $sorting_field_options,
  );
  $form['widget']['sorting_direction'] = array(
    '#type' => 'select',
    '#title' => t('Sort Direction'),
    '#default_value' => !empty($config['order']) ? $config['order'] : 'descending',
    '#options' => array(
      'ascending' => t('Ascending'),
      'descending' => t('Descending'),
    ),
  );
  $form['widget']['preview'] = array(
    '#type' => 'submit',
    '#value' => t('Preview'),
  );
  $form['widget']['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  
  $form['preview'] = array(
  	'#type' => 'container',
    '#attributes' => array(
      'class' => array('feed-widget-form-preview'),
    ),
  );
  $form['preview']['container'] = array(
    '#process' => array('form_process_feed_widget_preview'),
    '#markup' => '<div class="feed-widget-preview"></div>',
  );
  
  return $form;
}

/**
 * Feed Widget Form Submit Handler;
 */
function feed_widget_form_submit($form, &$form_state) {
  $feed = $form_state['#feed'];
  $feed_widget = $form_state['#feed_widget'];

  $configuration = array(
    'format' => array(),
    'show' => array(),
    'weight' => array(),
    'paginate' => !empty($form_state['values']['widget']['allow_pagination']),
    'limit' => $form_state['values']['widget']['limit'] > 1 ? $form_state['values']['widget']['limit'] : 5,
    'sort' => !empty($form_state['values']['widget']['sorting_field']) ? $form_state['values']['widget']['sorting_field'] : 'published',
    'order' => !empty($form_state['values']['widget']['sorting_direction']) ? $form_state['values']['widget']['sorting_direction'] : 'descending',
    'source' => url('feed/' . $feed->fid. '/format/json', array('absolute' => TRUE)),
    'random' => !empty($form_state['values']['widget']['random']),
  );
  
  if (!empty($form_state['values']['widget']['feed_fields'])) {
    $configuration['show']['infos'] = array();
    $configuration['format']['infos'] = array();
    $configuration['weight']['infos'] = array();
    
    uasort($form_state['values']['widget']['feed_fields'], 'drupal_sort_weight');

    foreach ($form_state['values']['widget']['feed_fields'] as $name => $value) {
      if (!empty($value['display']) && !in_array($name, $configuration['show']['infos'])) {
        $configuration['show']['infos'][] = $name;
      }
      if (!empty($value['format'])) {
        $configuration['format']['infos'][$name] = $value['format'];
      }
    }
  }
  
  if (!empty($form_state['values']['widget']['feed_item_fields'])) {
    $configuration['show']['items'] = array();
    $configuration['format']['items'] = array();
    $configuration['weight']['items'] = array();
    
    uasort($form_state['values']['widget']['feed_item_fields'], 'drupal_sort_weight');
    
    foreach ($form_state['values']['widget']['feed_item_fields'] as $name => $value) {
      if (!empty($value['display']) && !in_array($name, $configuration['show']['items'])) {
        $configuration['show']['items'][] = $name;
      }
      if (!empty($value['format'])) {
        $configuration['format']['items'][$name] = $value['format'];
      }
    }
  }

  $form_state['#configuration'] = $configuration;
  
  if ($form_state['values']['op'] == $form['widget']['preview']['#value']) {
    $form_state['rebuild'] = TRUE;
  }
  elseif ($form_state['values']['op'] == $form['widget']['save']['#value']) {
    $feed_widget->name = $form_state['values']['name'];
    $feed_widget->configuration = $form_state['#configuration'];
    $feed_widget->fid = $feed->fid;
    
    $script = feed_widget_javascript('#eosdis-feed-widget', $feed_widget->configuration, TRUE);
    $dirpath = FEED_WIDGET_FILE_CACHE_PATH;
    $filepath = $dirpath . '/' . (!empty($feed_widget->file) ? $feed_widget->file : uniqid('feed-widget-') . '.js');
    
    if (!is_dir($dirpath) && !file_prepare_directory($dirpath, FILE_CREATE_DIRECTORY)) {
      watchdog('feed', 'Could not create feed widget javascript cache directory.', array(), WATCHDOG_ERROR);
      drupal_set_message(t('Error creating feed widget javascript'), 'error');
    }
    elseif (!$feed_widget->file = file_unmanaged_save_data($script, $filepath, FILE_EXISTS_REPLACE)) {
      drupal_set_message(t('Error creating feed widget javascript'), 'error');
    }
    
    if (feed_widget_save($feed_widget)) {
      drupal_set_message(t('Successfully saved feed widget'), 'status');
    }
    else {
      drupal_set_message(t('Error saving feed widget'), 'error');
    }
    $form_state['redirect'] = 'feed/' . $feed->fid . '/widget/' . $feed_widget->fwid;
  }
}

/**
 * Generates Feed Widget javascript from configuration
 */
function feed_widget_javascript($selector = '#eosdis-feed-widget', $configuration, $compile = FALSE) {
  $script = theme('feed_widget', array(
    'selector' => $selector,
    'configuration' => $configuration,
  ));
  
  if ($compile && module_exists('packer') && is_callable('packer_compile')) {
    $script = packer_compile($script);
  }
  return $script;
}

/**
 * Process Feed Widget Preview
 */
function form_process_feed_widget_preview(&$element, &$form_state) {
  if (!empty($form_state['#configuration'])) {
    $element['#attached'] = array(
      'js' => array(
        drupal_get_path('module', 'feed') . '/js/feed.widget.js',
        array(
          'data' => '(function($){ $(function(){ new FeedWidgetInitialize(\'.feed-widget-preview\', Drupal.settings.feedwidgetPreview) }); })(jQuery);',
          'type' => 'inline',
        ),
        array(
          'data' => array('feedwidgetPreview' => $form_state['#configuration']),
          'type' => 'setting',
        )
      ),
    );
  }
  return $element;
}

/**
 * Delete feed widget form;
 */
function feed_widget_delete_form($form, &$form_state, $feed, $feed_widget) {
  $form_state['#feed_widget'] = feed_widget_defaults($feed_widget);

  $form['#attributes'] = array('class' => array('feed-form'));
  
  return confirm_form(
    $form,
    t('Delete feed widget?'),
    "feed/{$feed->fid}/widget",
    '<div class="messages warning"><span class="feed-warning">Warning:</span> ' . 
    t('You are about to delete feed widget ":name". This action cannot be undone', array(':name' => $feed_widget->name)) . '</div>',
    t('Delete'),  t('Cancel')
  );
}

/**
 * Delete feed widget form submit handler;
 */
function feed_widget_delete_form_submit(&$form, &$form_state) {
  $feed_widget = $form_state['#feed_widget'];
  
  if (feed_widget_delete($feed_widget)) {
    drupal_set_message(t('Feed widget :name was deleted.', array(':name' => $feed_widget->name)), 'status');
  } 
  else {
    drupal_set_message(t('Could not delete feed widget :name.', array(':name' => $feed_widget->name)), 'error');
  }
  $form_state['redirect'] = "feed/{$feed->fid}/widget";
}
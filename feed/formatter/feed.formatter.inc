<?php 

/**
 * Implements hook_feed_formatters().
 */
function feed_feed_formatters() {
  $formatters['rss'] = array(
    'label' => 'RSS',
    'file' => 'formatter/rss/feed-formatter.rss.inc',
  
    'feed_item_viewable' => FALSE,
  
    'format_feed' => 'feed_formatter_rss_format_feed',
    'format_feed_field' => 'feed_formatter_rss_format_feed_field',
    'format_feed_field_value' => 'feed_formatter_rss_format_feed_field_value',
  
    'format_feed_item' => 'feed_formatter_rss_format_feed_item',
    'format_feed_item_field' => 'feed_formatter_rss_format_feed_item_field',
    'format_feed_item_field_value' => 'feed_formatter_rss_format_feed_item_field_value',
  );
  $formatters['web'] = array(
    'label' => 'Web Preview',
    'file' => 'formatter/web/feed-formatter.web.inc',
  
    'feed_item_viewable' => TRUE,
  
    'feed_field_setting' => 'feed_formatter_web_feed_field_setting',
    'feed_field_setting_defaults' => array('label' => '',),
  
    'format_feed' => 'feed_formatter_web_format_feed',
    'format_feed_field' => 'feed_formatter_web_format_feed_field',
    'format_feed_field_value' => 'feed_formatter_web_format_feed_field_value',
  
    'format_feed_item' => 'feed_formatter_web_format_feed_item',
    'format_feed_item_field' => 'feed_formatter_web_format_feed_item_field',
    'format_feed_item_field_value' => 'feed_formatter_web_format_feed_item_field_value',
  );
  $formatters['json'] = array(
    'label' => 'JSON',
    'file' => 'formatter/json/feed-formatter.json.inc',
  
    'feed_item_viewable' => TRUE,
  
    'feed_field_setting' => 'feed_formatter_json_feed_field_setting',
    'feed_field_setting_defaults' => array('label' => '',),
  
    'format_feed' => 'feed_formatter_json_format_feed',
    'format_feed_field' => 'feed_formatter_json_format_feed_field',
    'format_feed_field_value' => 'feed_formatter_json_format_feed_field_value',
  
    'format_feed_item' => 'feed_formatter_json_format_feed_item',
    'format_feed_item_field' => 'feed_formatter_json_format_feed_item_field',
    'format_feed_item_field_value' => 'feed_formatter_json_format_feed_item_field_value',
  );
  return $formatters;
}

/**
 * Retrieves formatter infos
 */
function feed_formatter_info($formatter = NULL, $load = FALSE) {
  $feed_formatter_infos = &drupal_static(__FUNCTION__);
  
  if (!isset($feed_formatter_infos)) {
    if (false && $cache = cache_get('feed_formatter_info', 'cache')) {
      $feed_formatter_infos = $cache->data;
    }
    else {
      $feed_formatter_infos = array();
      foreach (module_implements('feed_formatters') as $module) {
        foreach (module_invoke($module, 'feed_formatters') as $name => $info) {
          $feed_formatter_infos[$name] = $info;
          $feed_formatter_infos[$name]['module'] = $module;
          $feed_formatter_infos[$name]['name'] = $name;
        }
      }
      cache_set('feed_formatter_info', $feed_formatter_infos);
    }
  }
  
  if ($formatter === NULL) {
    return $feed_formatter_infos;
  }
  elseif (isset($feed_formatter_infos[$formatter])) {
    $info = $feed_formatter_infos[$formatter];
    if ($load && isset($info['module']) && !isset($feed_formatter_infos[$formatter]['loaded'])) {
      $module = $info['module'];
      
      if (isset($info['file'])) {
        $file = drupal_get_path('module', $module) . DIRECTORY_SEPARATOR . $info['file'];
        
        if (file_exists($file)) {
          include_once $file;
        }
      }
      else {
        module_load_include('inc', $module);
      }
      $feed_formatter_infos[$formatter]['loaded'] = TRUE;
    }
    return $info;
  }
  return FALSE;
}

/**
 * Formatter method invoker; This function invokes the formatter methods
 * 
 * @param $type
 *   Formatter type as specified by the key in hook_feed_formatters implementations.
 * @param $method
 *   Method to call formatter function, can be a string or an array.
 * @param ...$arguments
 *   Every additional parameters will be passed as an argument to the method
 */
function feed_formatter_invoke($formatter, $method) {
  $args = func_get_args();
  $formatter = array_shift($args);
  $method = array_shift($args);
  $return = FALSE;
  
  if ($info = feed_formatter_info($formatter, TRUE)) {
    
    if (isset($info[$method]) && is_callable($info[$method])) {
      try {
        $return = call_user_func_array($info[$method], $args);
      }
      catch (Exception $e) {
        watchdog_exception('feed', $e);
      }
    }
    else {
      watchdog('feed', 'Unknown method to invoke for formatter :formatter', array(':formatter' => $formatter));
    }
  }
  else {
    watchdog('feed', 'Could not find feed formatter for type :formatter', array(':formatter' => $formatter));
  }
  return $return;
}

/**
 * Formats a feed from a feed object
 * 
 * @param $formatter
 *   A formatter type.
 * @param $feed
 *   A feed object.
 * @param $options
 *   An array of options.
 */
function feed_format_feed($formatter, $feed, array $options = array()) {
  if (!feed_formatter_info($formatter, TRUE)) {
    watchdog('feed', 'Unknown feed formatter :formatter', array(':formatter' => $formatter));
    return FALSE;
  }
  
  $defaults = array('page' => 1, 'limit' => 20, 'sort' => 'publish', 'order' => 'descending', 'random' => FALSE);
  
  if (!is_integer($options['page']) || $options['page'] <= 0) {
    $options['page'] = $defaults['page'];
  }
  if (!is_integer($options['limit']) || $options['limit'] <= 0) {
    $options['limit'] = $defaults['limit'];
  }
  if (!in_array($options['sort'], array('publish', 'created', 'title'))) {
    $options['sort'] = $defaults['sort'];
  }
  if (!in_array($options['order'], array('descending', 'ascending'))) {
    if (!empty($options['sort']) && $options['sort'] == 'title') {
      $options['order'] = 'ascending';
    }
    else {
      $options['order'] = $defaults['order'];
    }
  }

  if (!empty($options['random']) || (empty($options['random']) && $defaults['random'])) {
    $options['random'] = '1';
  }
  else {
    unset($options['random']);
  }
  
  $pager = new FeedItemPager();
  $pager->hasFeedCreated($feed->fid);
  $pager->hasFeedSubscribed($feed->fid);
  $pager->setCurrentPage($options['page']);
  $pager->setItemLimit($options['limit']);
  $pager->hasFieldValueByTagname('pubDate', array('timestamp' => '0'), '>');
  
  $pager->addFieldValueColumnByTagname('title', 'text', 'title');
  $pager->addFieldValueColumnByTagname('description', 'text', 'description');
  $pager->addFieldValueColumnByTagname('pubDate', 'timestamp', 'publish');
  
  $query = $pager->build();

  if (!empty($options['random'])) {
    $query = db_select($query->orderRandom(), 'random');
    $query->fields('random');
  }
  
  if (!empty($options['sort'])) {
    $query->orderBy($options['sort']);
  }
  $feed_items = $query->execute()->fetchAllAssoc('fiid');
  
  $feed_fields = feed_load_fields($feed, FALSE);
  $feed_item_fields = feed_load_fields($feed, TRUE);
  
  $formatted_feed_fields = array();
  $formatted_feed_items = array();

  // Generate each feed field format.
  foreach ($feed_fields as $feed_field) {
    $feed_field_values = feed_field_values_load($feed, $feed_field);
    
    $formatted_feed_field_values = array();
    foreach ($feed_field_values as $feed_field_value) {
      $formatted_feed_field_values[$feed_field_value->ffvid] = feed_format_feed_field_value($formatter, $feed, $feed_field, $feed_field_value);
    }
    
    $formatted_feed_field = feed_format_feed_field($formatter, $feed, $feed_fields[$feed_field->ffid], array('#content' => $formatted_feed_field_values));
    $formatted_feed_fields[$feed_field->ffid] = $formatted_feed_field;
  }
  
  // Generate each feed item format.
  foreach ($feed_items as &$feed_item) {
    if ($feed_item->fid == $feed->fid) {
      $feed_item->type = FEED_TYPE_CREATED;
      $formatted_feed_items[] = feed_format_feed_item($formatter, $feed_item, $feed_item_fields, FALSE);
    }
    else {
      $feed_item->type = FEED_TYPE_SUBSCRIBED;
      $formatted_feed_items[] = feed_format_feed_item($formatter, $feed_item, NULL, FALSE);
    }
  }
  
  $variables['#feed'] = $feed;
  $variables['#content'] = array();
  $variables['#content']['feed_fields'] = $formatted_feed_fields;
  $variables['#content']['feed_items'] = $formatted_feed_items;
  
  $variables['#query'] = array();
  $variables['#query']['query'] = $pager;
  $variables['#query']['params'] = $options;
  $variables['#query']['pagination'] = array(
    'page_num' => $pager->getCurrentPage(),
    'page_max' => $pager->getMaxPage(),
  );
  
  return feed_formatter_invoke($formatter, 'format_feed', $variables);
}

/**
 * Formats feed fields from an array of feed field objects
 * 
 * @param $formatter
 *   A formatter type.
 * @param $feed
 *   A feed object.
 * @param $feed_field
 *   A feed field object.
 * @param $variables
 *   An array of variables to pass to formatter
 */
function feed_format_feed_field($formatter, $feed, $feed_field, $variables = array()) {
  $variables['#feed'] = $feed;
  $variables['#feed_field'] = $feed_field;
  
  return feed_formatter_invoke($formatter, 'format_feed_field', $variables);
}

/**
 * Formats feed field values from an array of feed field value objects
 * 
 * @param $formatter
 *   A formatter type.
 * @param $feed
 *   A feed object.
 * @param $feed_field
 *   A feed field object.
 * @param $feed_field_value
 *   An array of feed field value objects.
 * @param $variables
 *   An array of variables to pass to formatter
 */
function feed_format_feed_field_value($formatter, $feed, $feed_field, $feed_field_value, $variables = array()) {
  $variables['#feed'] = $feed;
  $variables['#feed_field'] = $feed_field;
  $variables['#feed_field_value'] = $feed_field_value;
  
  return feed_formatter_invoke($formatter, 'format_feed_field_value', $variables);
}

/**
 * Formats a feed item from a feed item object
 * 
 * @param $formatter
 *   A formatter type.
 * @param $feed_item
 *   A feed item object.
 * @param $feed_item_fields
 *   An array of feed field objects to format
 */
function feed_format_feed_item($formatter, $feed_item, $feed_item_fields = NULL, $view = TRUE) {
  $variables['#feed_item'] = $feed_item;
  $variables['#feed_item_view'] = $view;
  
  if (empty($feed_item_fields)) {
    $feed_item_fields = feed_item_fields_load($feed_item);
  }
  
  $variables['#content'] = array();
  if (is_array($feed_item_fields)) {
    foreach ($feed_item_fields as $feed_item_field) {
      $formatted_feed_item_field_values = array();
      foreach (feed_item_field_values_load($feed_item, $feed_item_field) as $feed_item_field_value) {
        $formatted_feed_item_field_values[$feed_item_field_value->fifvid] = feed_format_feed_item_field_value($formatter, $feed_item, $feed_item_field, $feed_item_field_value);
      }
      $formatted_feed_item_field = feed_format_feed_item_field($formatter, $feed_item, $feed_item_field, array('#content' => $formatted_feed_item_field_values));
      $variables['#content'][$feed_item_field->ffid] = $formatted_feed_item_field;
    }
  }
  return feed_formatter_invoke($formatter, 'format_feed_item', $variables);
}

/**
 * Formats feed item fields from an array of feed field objects
 * 
 * @param $formatter
 *   A formatter type.
 * @param $feed_item
 *   A feed item object.
 * @param $feed_item_fields
 *   An array of feed field objects.
 * @param $variables
 *   An array of variables to pass to formatter
 */
function feed_format_feed_item_field($formatter, $feed_item, $feed_item_field, $variables = array()) {
  $variables['#feed_item'] = $feed_item;
  $variables['#feed_item_field'] = $feed_item_field;
  
  return feed_formatter_invoke($formatter, 'format_feed_item_field', $variables);
}

/**
 * Formats feed item field values from an array of feed item field value objects
 * 
 * @param $formatter
 *   A formatter type.
 * @param $feed_item
 *   A feed item object.
 * @param $feed_item_field
 *   A feed item field object.
 * @param $feed_item_field_values
 *   An array of feed item field value objects.
 * @param $variables
 *   An array of variables to pass to formatter
 */
function feed_format_feed_item_field_value($formatter, $feed_item, $feed_item_field, $feed_item_field_value, $variables = array()) {
  $variables['#feed_item'] = $feed_item;
  $variables['#feed_item_field'] = $feed_item_field;
  $variables['#feed_item_field_value'] = $feed_item_field_value;
  
  return feed_formatter_invoke($formatter, 'format_feed_item_field_value', $variables);
}
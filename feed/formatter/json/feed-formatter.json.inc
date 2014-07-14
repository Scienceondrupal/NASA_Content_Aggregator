<?php 

module_load_include('inc', 'feed', 'formatter/json/feed-formatter-setting.json');

/**
 * Returns the query parameter for json options
 * 
 * @param $options
 */
function feed_formatter_json_option($option) {
  static $query_params;
  if (!isset($query_params)) {
    $query_params = drupal_get_query_parameters();
  }
  return isset($query_params[$option]) ? $query_params[$option] : FALSE;
}

/**
 * Returns the query parameter for json format options
 * 
 * @param $type
 * @param $tagname
 */
function feed_formatter_json_format_option($type, $tagname) {
  if ($option = feed_formatter_json_option('format')) {
    if (is_array($option) && !empty($option[$type][$tagname])) {
      return $option[$type][$tagname];
    }
  }
  return FALSE;
}

/**
 * Returns the query parameter for json show options
 */
function feed_formatter_json_show_option($type, $tagname) {
  if ($option = feed_formatter_json_option('show')) {
    if (is_array($option[$type])) {
      return in_array($tagname, $option[$type]);
    }
  }
  return FALSE;
}

/**
 * Format Feed Callback;
 */
function feed_formatter_json_format_feed($variables) {
  $feed =& $variables['#feed'];
  $content = array();
  $fields = feed_formatter_json_option('show');
  
  $content['infos'] = array();
  foreach ($fields['infos'] as $name) {
    $content['infos'][$name] = FALSE;
  }
  
  if (is_array($variables['#content']['feed_fields'])) {
    $content['infos'] = array();
    foreach (array_filter($variables['#content']['feed_fields']) as $feed_fields) {
      foreach ($feed_fields as $name => $feed_field) {
        $feed_field = array_filter($feed_field);
        
        if (!empty($feed_field)) {
          $content['infos'][$name] = $feed_field;
        }
      }
    }
  }
  array_filter($content['infos']);
  
  if (is_array($variables['#content']['feed_items'])) {
    $content['items'] = $variables['#content']['feed_items'];
  }
  
  if (!empty($variables['#query']['pagination'])) {
    $current_page = $variables['#query']['pagination']['page_num'];
    $max_page = $variables['#query']['pagination']['page_max'];
    $previous_page = (($current_page - 1) > 0) ? $current_page - 1 : FALSE;
    $next_page = (($current_page + 1) <= $max_page) ? $current_page + 1 : FALSE;
    
    $content['pages'] = array(
      'current'      => $current_page,
      'previous'   => $previous_page,
      'next'       => $next_page,
      'pages'      => $max_page,
    );
  }

  if (isset($_GET['callback'])) {
    drupal_add_http_header('Content-Type', 'application/javascript');
    print sprintf('%s(%s);', check_plain($_GET['callback']), json_encode($content));
  }
  else {
    pr($content);
    drupal_json_output($content);
  }
  exit();
}

/**
 * Format Feed Field Callback;
 */
function feed_formatter_json_format_feed_field($variables) {
  $feed = $variables['#feed'];
  $feed_field = $variables['#feed_field'];
  
  $content = array_filter($variables['#content']);
  
  if (!empty($content)) {
    if ($feed_field->namespace == XmlElement::DEFAULT_NAMESPACE) {
      $field_tag = $feed_field->tag;
    }
    elseif (!empty($feed_field->namespace)) {
      $field_tag = $feed_field->namespace . ':' . $feed_field->tag;
    }
    return array($field_tag => array_values($content));
  }
  return FALSE;
}

/**
 * Format Feed Field Value Callback;
 */
function feed_formatter_json_format_feed_field_value($variables) {
  $feed = $variables['#feed'];
  $feed_field = $variables['#feed_field'];
  $feed_field_value = $variables['#feed_field_value'];
  
  if (isset($feed_field->data[FEED_DATA_FEED_FIELD_DISPLAY_ENABLED]['json']) && empty($feed_field->data[FEED_DATA_FEED_FIELD_DISPLAY_ENABLED]['json'])) {
    return FALSE;
  }
  if ($feed_field->namespace == XmlElement::DEFAULT_NAMESPACE) {
    $field_tag = $feed_field->tag;
  }
  elseif (!empty($feed_field->namespace)) {
    $field_tag = $feed_field->namespace . ':' . $feed_field->tag;
  }
  $show = feed_formatter_json_show_option('infos', $field_tag);
  
  if ($show && $option = feed_formatter_json_format_option('infos', $field_tag)) {
    return feed_field_value_format($feed_field_value, $option);
  }
  return FALSE;
}

/**
 * Format Feed Item Callback;
 */
function feed_formatter_json_format_feed_item($variables) {
  $feed_item = $variables['#feed_item'];
  
  $fields = feed_formatter_json_option('show');
  
  $content = array();
  foreach ($fields['items'] as $name) {
    $content[$name] = FALSE;
  }
  if (is_array($variables['#content'])) {
    foreach (array_filter($variables['#content']) as $feed_item_fields) {
      foreach ($feed_item_fields as $name => $feed_item_field) {
        $feed_item_field = array_filter($feed_item_field);
        
        if (!empty($feed_item_field)) {
          $content[$name] = $feed_item_field;
        }
      }
    }
  }
  array_filter($content);
  
  if (!empty($variables['#feed_item_view'])) {
    if (isset($_GET['callback'])) {
      drupal_add_http_header('Content-Type', 'application/javascript');
      print sprintf('%s(%s);', check_plain($_GET['callback']), json_encode($content));
    }
    else {
      drupal_json_output($content);
    }
    exit();
  }
  return $content;
}

/**
 * Format Feed Item Field Callback;
 */
function feed_formatter_json_format_feed_item_field($variables) {
  $feed_item = $variables['#feed_item'];
  $feed_item_field = $variables['#feed_item_field'];
  
  $content = array_filter($variables['#content']);
  
  if (!empty($content)) {
    if ($feed_item_field->namespace == XmlElement::DEFAULT_NAMESPACE) {
      $field_tag = $feed_item_field->tag;
    }
    elseif (!empty($feed_item_field->namespace)) {
      $field_tag = $feed_item_field->namespace . ':' . $feed_item_field->tag;
    }
    return array($field_tag => array_values($content));
  }
  return FALSE;
}

/**
 * Format Feed Item Field Value Callback;
 */
function feed_formatter_json_format_feed_item_field_value($variables) {
  $feed_item = $variables['#feed_item'];
  $feed_item_field = $variables['#feed_item_field'];
  $feed_item_field_value = $variables['#feed_item_field_value'];
  
  if (isset($feed_item_field->data[FEED_DATA_FEED_FIELD_DISPLAY_ENABLED]['json']) && empty($feed_item_field->data[FEED_DATA_FEED_FIELD_DISPLAY_ENABLED]['json'])) {
    return FALSE;
  }
  if ($feed_item_field->namespace == XmlElement::DEFAULT_NAMESPACE) {
    $field_tag = $feed_item_field->tag;
  }
  elseif (!empty($feed_item_field->namespace)) {
    $field_tag = $feed_item_field->namespace . ':' . $feed_item_field->tag;
  }
  $show = feed_formatter_json_show_option('items', $field_tag);
  
  if ($show && $option = feed_formatter_json_format_option('items', $field_tag)) {
    return feed_item_field_value_format($feed_item_field_value, $option);
  }
  return FALSE;
}
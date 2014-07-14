<?php 

module_load_include('inc', 'feed', 'formatter/json/feed-formatter-setting.rss');

/**
 * Format Feed Callback;
 */
function feed_formatter_rss_format_feed($variables) {
  global $base_url;
  
  $feed = $variables['#feed'];
  
  $rss_element = new XmlElement('rss');
  $rss_element->setAttribute('version', '2.0');
  $rss_element->setAttribute('xml:base', $base_url);
  
  $namespaces = feed_object_get_data($feed, FEED_DATA_FEED_NAMESPACE, array());
  
  if (!empty($namespaces) && is_array($namespaces)) {
    foreach ($namespaces as $name => $location) {
      $rss_element->addNamespace($name, $location);
    }
  }
  $rss_element->addChild($channel_element = new XmlElement('channel'));
  
  if (!empty($variables['#content']['feed_fields'])) {
    foreach ($variables['#content']['feed_fields'] as $feed_field_content) {
      if ($feed_field_content instanceof XmlElements) {
        $channel_element->addChildren($feed_field_content);
      }
    }
  }
  
  if (!empty($variables['#content']['feed_items'])) {
    foreach ($variables['#content']['feed_items'] as $feed_item_content) {
      if ($feed_item_content instanceof XmlElement) {
        $channel_element->addChild($feed_item_content);
      }
    }
  }
  
  drupal_add_http_header('Content-Type', 'application/rss+xml; charset=utf-8');
  
  print '<?xml version="1.0" encoding="utf-8" ?>' . XML_CHAR_CRLF . $rss_element->render();

  exit();
}

/**
 * Format Feed Field Callback;
 */
function feed_formatter_rss_format_feed_field($variables) {
  $feed = $variables['#feed'];
  $feed_field = $variables['#feed_field'];
  
  if (!empty($variables['#content'])) {
    $elements = new XmlElements();
    
    foreach ($variables['#content'] as $element) {
      if ($element instanceof XmlElement) {
        $elements->addElement($element);
      }
    }
    return $elements;
  }
  return FALSE;
}

/**
 * Format Feed Field Value Callback;
 */
function feed_formatter_rss_format_feed_field_value($variables) {
  $feed = $variables['#feed'];
  $feed_field = $variables['#feed_field'];
  $feed_field_value = $variables['#feed_field_value'];
  
  if (!isset($feed_field->data[FEED_DATA_FEED_FIELD_DISPLAY_ENABLED]['rss']) || !empty($feed_field->data[FEED_DATA_FEED_FIELD_DISPLAY_ENABLED]['rss'])) {
    return feed_field_value_format($feed_field_value, 'rss');
  }
  return FALSE;
}

/**
 * Format Feed Item Callback;
 */
function feed_formatter_rss_format_feed_item($variables) {
  $feed_item = $variables['#feed_item'];
  
  $item_element = new XmlElement('item');
  
  if (!empty($variables['#content'])) {
    foreach ($variables['#content'] as $elements) {
      if ($elements instanceof XmlElements && $elements->count()) {
        foreach ($elements as $element) {
          $item_element->addChild($element);
        }
      }
    }
  }
  return $item_element;
}

/**
 * Format Feed Item Field Callback;
 */
function feed_formatter_rss_format_feed_item_field($variables) {
  $feed_item = $variables['#feed_item'];
  $feed_item_field = $variables['#feed_item_field'];
  
  if (!empty($variables['#content']) && is_array($variables['#content'])) {
    $elements = new XmlElements();
    
    foreach ($variables['#content'] as $element) {
      if ($element instanceof XmlElement) {
        $elements->addElement($element);
      }
    }
    return $elements;
  }
  return FALSE;
}

/**
 * Format Feed Item Field Value Callback;
 */
function feed_formatter_rss_format_feed_item_field_value($variables) {
  $feed_item = $variables['#feed_item'];
  $feed_item_field = $variables['#feed_item_field'];
  $feed_item_field_value = $variables['#feed_item_field_value'];
  
  if (!isset($feed_item_field->data[FEED_DATA_FEED_FIELD_DISPLAY_ENABLED]['rss']) || !empty($feed_item_field->data[FEED_DATA_FEED_FIELD_DISPLAY_ENABLED]['rss'])) {
    return feed_item_field_value_format($feed_item_field_value, 'rss');
  }
  return FALSE;
}
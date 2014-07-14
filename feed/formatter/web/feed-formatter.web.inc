<?php 

module_load_include('inc', 'feed', 'formatter/web/feed-formatter-setting.web');

/**
 * Theme wrapper for web formatter feed field
 * 
 * @param $variables
 */
function theme_feed_web_format_field($variables) {
  $element = $variables['element'] + array('#attributes' => array());
  
  $attributes = array_merge_recursive($element['#attributes'], array(
    'class' => array('feed-web-format-field'),
  ));
  
  $output = '<div' . drupal_attributes($attributes) . '>';
  if (isset($element['#prefix'])) {
    $output .= $element['#prefix'];
  }
  $output .= render($element['#content']);
  if (isset($element['#suffix'])) {
    $output .= $element['#suffix'];
  }
  $output .= '</div>';
  return $output;
}

/**
 * Format Feed Callback;
 */
function feed_formatter_web_format_feed($variables) {
  drupal_add_css(drupal_get_path('module', 'feed') . '/formatter/web/feed-formatter.web.css');
  
  if (isset($variables['#query']['pagination']['page_num'], $variables['#query']['pagination']['page_max'])) {
    $page_num = $variables['#query']['pagination']['page_num'];
    $page_max = $variables['#query']['pagination']['page_max'];
    
    $variables['#query']['pagination']['page_end'] = $page_max > ($page_num + 5) ? ($page_num + 5) : $page_max;
    $variables['#query']['pagination']['page_start'] = $page_num > 6 ? ($page_num - 5) : 1;
  }
  
  $element = array(
    'feed_fields' => $variables['#content']['feed_fields'],
    'feed_items' => $variables['#content']['feed_items'],
    'params' => $variables['#query']['params'],
    'pagination' => $variables['#query']['pagination'],
  );
  
  if ($title = feed_field_values_tagname_format($variables['#feed']->fid, 'title', 'text')) {
    drupal_set_title($title);
  }
  
  return theme('feed_web_formatter', array('element' => $element));
}

/**
 * Format Feed Field Callback;
 */
function feed_formatter_web_format_feed_field($variables) {
  $feed = $variables['#feed'];
  $feed_field = $variables['#feed_field'];
  
  $content = array();
  $content['#title'] = $feed_field->name;
  $content['feed_field_values'] = $variables['#content'];
  
  return $content;
}

/**
 * Format Feed Field Value Callback;
 */
function feed_formatter_web_format_feed_field_value($variables) {
  $feed = $variables['#feed'];
  $feed_field = $variables['#feed_field'];
  $feed_field_value = $variables['#feed_field_value'];
  
  if (!isset($feed_field->data[FEED_DATA_FEED_FIELD_DISPLAY_ENABLED]['web']) || !empty($feed_field->data[FEED_DATA_FEED_FIELD_DISPLAY_ENABLED]['web'])) {
    return feed_field_value_format($feed_field_value, 'web');
  }
  return FALSE;
}

/**
 * Format Feed Item Callback;
 */
function feed_formatter_web_format_feed_item($variables) {
  $feed_item = $variables['#feed_item'];
  
  $allowed = array('a', 'em', 'strong', 'cite', 'blockquote', 'code', 'ul', 'ol', 'li', 'dl', 'dt', 'dd', 'img');

  $element = array(
    'feed_item_fields' => $variables['#content'],
  );
  return theme('feed_item_web_formatter', array('element' => $element));
}

/**
 * Format Feed Item Field Callback;
 */
function feed_formatter_web_format_feed_item_field($variables) {
  $feed_item = $variables['#feed_item'];
  $feed_item_field = $variables['#feed_item_field'];
  
  $content = array();
  $content['#title'] = $feed_item_field->name;
  $content['feed_item_field_values'] = $variables['#content'];
  
  return $content;
}

/**
 * Format Feed Item Field Value Callback;
 */
function feed_formatter_web_format_feed_item_field_value($variables) {
  $feed_item = $variables['#feed_item'];
  $feed_item_field = $variables['#feed_item_field'];
  $feed_item_field_value = $variables['#feed_item_field_value'];

  if (!isset($feed_item_field->data[FEED_DATA_FEED_FIELD_DISPLAY_ENABLED]['web']) || !empty($feed_item_field->data[FEED_DATA_FEED_FIELD_DISPLAY_ENABLED]['web'])) {
    return feed_item_field_value_format($feed_item_field_value, 'web');
  }
  return FALSE;
}
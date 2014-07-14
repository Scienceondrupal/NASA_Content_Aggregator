<?php

/**
 * Feed field settings for JSON formatter
 * 
 * @param $feed_field
 *   A feed field object.
 * @param $settings
 *   An array of settings
 *   
 * @return
 *   A drupal form element.
 */
function feed_formatter_json_feed_field_setting($feed_field, $settings) {
  $element['label'] = array(
    '#type' => 'textfield',
    '#title' => t('Label for JSON formatter'),
    '#default_value' => !empty($settings['label']) ? $settings['label'] : $feed_field->name,
    '#description' => t('The displayed title for this field in a JSON output'),
  );
  
  return $element;
}
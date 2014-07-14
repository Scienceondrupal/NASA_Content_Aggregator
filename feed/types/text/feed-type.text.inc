<?php

/**
 * Feed field formatter; Formats a text field for json output.
 */
function feed_field_text_format_json($feed_field, $feed_field_value, $field_instance, $formatter_instance, $values) {
  $json = array();
  $json['html'] = feed_field_text_format_web($feed_field, $feed_field_value, $field_instance, $formatter_instance, $values);
  
  $json['values']['type'] = 'text';
  $json['values']['text'] = check_plain($values['text']);
  $json['attributes'] = feed_object_get_data($feed_field_value, FEED_DATA_TYPE_FIELD_ATTRIBUTE, array());
  return $json;
}

/**
 * Feed field formatter; Formats a text field for rss output.
 */
function feed_field_text_format_rss($feed_field, $feed_field_value, $field_instance, $formatter_instance, $values) {
  $element = new XmlElement($feed_field->tag, $feed_field->namespace);
  
  if (!empty($values['text']) && is_string($values['text'])) {
    $element->setValue($values['text']);
  }
  $element->setNamespacedAttributes(feed_object_get_data($feed_field_value, FEED_DATA_TYPE_FIELD_ATTRIBUTE, array()));
  return $element;
}

/**
 * Feed field formatter; Formats a text field for text output.
 */
function feed_field_text_format_text($feed_field, $feed_field_value, $field_instance, $formatter_instance, $values) {
  return $values['text'];
}

/**
 * Feed field formatter; Formats a text field for web output.
 */
function feed_field_text_format_web($feed_field, $feed_field_value, $field_instance, $formatter_instance, $values) {
  return '<div class="feed-field-text">' . check_plain($values['text']) . '</div>';
}

/**
 * Feed field formatter; Formats a text field for json output.
 */
function feed_item_field_text_format_json($feed_field, $feed_item_field_value, $field_instance, $formatter_instance, $values) {
  $json = array();
  $json['html'] = feed_item_field_text_format_web($feed_field, $feed_item_field_value, $field_instance, $formatter_instance, $values);
  
  $json['values']['type'] = 'text';
  $json['values']['text'] = check_plain($values['text']);
  $json['attributes'] = feed_object_get_data($feed_item_field_value, FEED_DATA_TYPE_FIELD_ATTRIBUTE, array());
  return $json;
}

/**
 * Feed item field formatter; Formats a text field for rss output.
 */
function feed_item_field_text_format_rss($feed_field, $feed_item_field_value, $field_instance, $formatter_instance, $values) {
  $element = new XmlElement($feed_field->tag, $feed_field->namespace);
  
  if (!empty($values['text']) && is_string($values['text'])) {
    $element->setValue($values['text']);
  }
  $element->setNamespacedAttributes(feed_object_get_data($feed_item_field_value, FEED_DATA_TYPE_FIELD_ATTRIBUTE, array()));
  return $element;
}

/**
 * Feed item field formatter; Formats a text field for text output.
 */
function feed_item_field_text_format_text($feed_field, $feed_item_field_value, $field_instance, $formatter_instance, $values) {
  return $values['text'];
}

/**
 * Feed item field formatter; Formats a text field for web output.
 */
function feed_item_field_text_format_web($feed_field, $feed_item_field_value, $field_instance, $formatter_instance, $values) {
  return '<div class="feed-item-field-text">' . check_plain($values['text']) . '</div>';
}

/**
 * Feed field widget callback; Creates a text feed field widget element
 */
function feed_field_text_widget($feed_field, $instance, $values) {
  
  $form['text'] = array(
    '#type' => 'textfield',
    '#maxlength' => $instance['maxlength'],
    '#size' => $instance['size'],
    '#default_value' => !empty($values['text']) ? $values['text'] : '',
    '#title' => !empty($instance['title']) ? $instance['title'] : t(ucfirst($feed_field->name)),
    '#required' => !empty($instance['required']),
    '#description' => !empty($instance['description']) ? $instance['description'] : NULL,
  );
  
  return $form;
}

/**
 * Feed field submit callback; Returns data to be stored for widget
 */
function feed_field_text_widget_submit($feed_field, $instance, $values) {
  $data = array();
  $data['text'] = !empty($values['text']) ? $values['text'] : '';
  
  return $data;
}

/**
 * Feed field settings callback; Creates a text feed field settings element
 */
function feed_field_text_settings($feed_field, $instance) {
  
  $form['maxlength'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum length'),
    '#default_value' => $instance['maxlength'],
    '#required' => FALSE,
    '#description' => t('The maximum length of the field in characters.'),
    '#element_validate' => array('feed_field_validate_integer_positive'),
  );
  
  $form['size'] = array(
    '#type' => 'textfield',
    '#title' => t('Size of the text field'),
    '#default_value' => $instance['size'],
    '#required' => FALSE,
    '#element_validate' => array('feed_field_validate_integer_positive'),
  );
  
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Label of the text field'),
    '#default_value' => isset($instance['title']) ? $instance['title'] : t(ucfirst($feed_field->name)),
    '#required' => FALSE,
  );
  
  $form['description'] = array(
    '#type' => 'textfield',
    '#title' => t('Description of the text field'),
    '#default_value' => isset($instance['description']) ? $instance['description'] : t('Text value for ' . $feed_field->name),
    '#required' => FALSE,
  );
  return $form;
}
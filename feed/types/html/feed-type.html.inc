<?php

/**
 * Feed field formatter; Formats a text field for json output.
 */
function feed_field_html_format_json($feed_field, $feed_field_value, $field_instance, $formatter_instance, $values) {
  $json = array();
  $json['html'] = feed_field_html_format_web($feed_field, $feed_field_value, $field_instance, $formatter_instance, $values);
  
  $json['values']['type'] = 'html';
  $json['values']['text'] = check_plain($values['text']);
  $json['attributes'] = feed_object_get_data($feed_field_value, FEED_DATA_TYPE_FIELD_ATTRIBUTE, array());
  return $json;
}

/**
 * Feed field formatter; Formats a html field for rss output.
 */
function feed_field_html_format_rss($feed_field, $feed_field_value, $field_instance, $formatter_instance, $values) {
  $element = new XmlElement($feed_field->tag, $feed_field->namespace);
  
  if (!empty($values['html']) && is_string($values['html'])) {
    $element->setValue($values['html']);
  }
  $element->setNamespacedAttributes(feed_object_get_data($feed_field_value, FEED_DATA_TYPE_FIELD_ATTRIBUTE, array()));
  return $element;
}

/**
 * Feed field formatter; Formats a html field for text output.
 */
function feed_field_html_format_text($feed_field, $feed_field_value, $field_instance, $formatter_instance, $values) {
  return drupal_html_to_text($values['html'], array());
}

/**
 * Feed field formatter; Formats a html field for web output.
 */
function feed_field_html_format_web($feed_field, $feed_field_value, $field_instance, $formatter_instance, $values) {
  return '<div class="feed-field-html">' . $values['html'] . '</div>';
}

/**
 * Feed item field formatter; Formats a text field for json output.
 */
function feed_item_field_html_format_json($feed_field, $feed_item_field_value, $field_instance, $formatter_instance, $values) {
  $json = array();
  $json['html'] = feed_item_field_html_format_web($feed_field, $feed_item_field_value, $field_instance, $formatter_instance, $values);
  
  $json['values']['type'] = 'html';
  $json['values']['text'] = $values['text'];
  $json['attributes'] = feed_object_get_data($feed_item_field_value, FEED_DATA_TYPE_FIELD_ATTRIBUTE, array());
  return $json;
}

/**
 * Feed item field formatter; Formats a html field for rss output.
 */
function feed_item_field_html_format_rss($feed_field, $feed_item_field_value, $field_instance, $formatter_instance, $values) {
  $element = new XmlElement($feed_field->tag, $feed_field->namespace);
  
  if (!empty($values['html']) && is_string($values['html'])) {
    $element->setValue($values['html']);
  }
  $element->setNamespacedAttributes(feed_object_get_data($feed_item_field_value, FEED_DATA_TYPE_FIELD_ATTRIBUTE, array()));
  return $element;
}

/**
 * Feed item field formatter; Formats a html field for text output.
 */
function feed_item_field_html_format_text($feed_item_field, $feed_item_field_value, $field_instance, $formatter_instance, $values) {
  return drupal_html_to_text($values['html'], array());
}

/**
 * Feed item field formatter; Formats a html field for web output.
 */
function feed_item_field_html_format_web($feed_item_field, $feed_item_field_value, $field_instance, $formatter_instance, $values) {
  return '<div class="feed-item-field-html">' . $values['html'] . '</div>';
}

/**
 * Feed field widget callback; Creates a html feed field widget element
 */
function feed_field_html_widget($feed_field, $instance, $values) {
  
  $form['html'] = array(
    '#type' => 'text_format',
    '#default_value' => !empty($values['html']) ? $values['html'] : '',
    '#title' => !empty($instance['title']) ? $instance['title'] : t(ucfirst($feed_field->name)),
    '#description' => !empty($instance['description']) ? $instance['description'] : NULL,
    '#format' => !empty($values['format']) ? $values['format'] : 'wysiwyg',
    '#required' => !empty($instance['required']),
  );
  
  return $form;
}

/**
 * Feed field submit callback; Returns data to be stored for widget
 */
function feed_field_html_widget_submit($feed_field, $instance, $values) {
  $allowed = array();
  if (!empty($instance['allowed'])) {
    foreach (preg_split('/\s+/', $instance['allowed']) as $allowed_part) {
      if (preg_match('/[a-z0-9:]+/i', $allowed_part)) {
        $allowed[] = $allowed_part;
      }
    }
  }
  $data = array();
  
  if (is_array($values['html'])) {
    $data['html'] = filter_xss(!empty($values['html']['value']) ? $values['html']['value'] : '', $allowed);
    $data['format'] = !empty($values['html']['format']) ? $values['html']['format'] : '';
  }
  elseif (is_string($values['html'])) {
    $data['html'] = filter_xss(!empty($values['html']) ? $values['html'] : '', $allowed);
  }
  
  if (!empty($data['html'])) {
    $data['text'] = drupal_html_to_text($data['html']);
  }
  return $data;
}

/**
 * Feed field settings callback; Creates a html feed field settings element
 */
function feed_field_html_settings($feed_field, $instance) {
  
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Label of the HTML field'),
    '#default_value' => isset($instance['title']) ? $instance['title'] : t(ucfirst($feed_field->name)),
    '#required' => FALSE,
  );
  
  $form['description'] = array(
    '#type' => 'textfield',
    '#title' => t('Description of the HTML field'),
    '#default_value' => isset($instance['description']) ? $instance['description'] : t('HTML for ' . $feed_field->name),
    '#required' => FALSE,
  );
  
  $form['allowed'] = array(
    '#type' => 'textfield',
    '#title' => t('List of allowed HTML tags in field, separate each tag by a space'),
    '#default_value' => isset($instance['allowed']) ? $instance['allowed'] : '',
    '#required' => FALSE,
  );
  return $form;
}
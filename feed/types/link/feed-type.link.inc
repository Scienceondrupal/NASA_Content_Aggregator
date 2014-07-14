<?php

/**
 * Feed field formatter; Formats a link field for json output.
 */
function feed_field_link_format_json($feed_field, $feed_field_value, $field_instance, $formatter_instance, $values) {
  $json = array();
  $json['html'] = feed_field_link_format_web($feed_field, $feed_field_value, $field_instance, $formatter_instance, $values);
  
  $json['values']['type'] = 'link';
  $json['values']['title'] = check_plain($values['label']);
  $json['values']['url'] = check_url($values['url']);
  $json['attributes'] = feed_object_get_data($feed_field_value, FEED_DATA_TYPE_FIELD_ATTRIBUTE, array());
  return $json;
}

/**
 * Feed field formatter; Formats a text field for rss output.
 */
function feed_field_link_format_rss($feed_field, $feed_field_value, $field_instance, $formatter_instance, $values) {
  $element = new XmlElement($feed_field->tag, $feed_field->namespace);
  
  if (!empty($values['url']) && is_string($values['url'])) {
    $element->setValue($values['url']);
  }
  $element->setNamespacedAttributes(feed_object_get_data($feed_field_value, FEED_DATA_TYPE_FIELD_ATTRIBUTE, array()));
  return $element;
}

/**
 * Feed field formatter; Formats a link field for text output.
 */
function feed_field_link_format_text($feed_field, $feed_field_value, $field_instance, $formatter_instance, $values) {
  return url($values['url'], array('absolute' => TRUE));
}

/**
 * Feed field formatter; Formats a link field for link output.
 */
function feed_field_link_format_web($feed_field, $feed_field_value, $field_instance, $formatter_instance, $values) {
  if (!empty($values['label'])) {
    return l($values['label'], $values['url'], array(
      'attributes' => array('class' => 'feed-field-link'),
      'title' => $values['label'],
      'absolute' => TRUE,
    ));
  }
  else {
    return l($values['url'], $values['url'], array(
      'attributes' => array('class' => 'feed-field-link'),
      'title' => $values['url'],
      'absolute' => TRUE,
    ));
  }
}

/**
 * Feed field formatter; Formats a link field for json output.
 */
function feed_item_field_link_format_json($feed_field, $feed_item_field_value, $field_instance, $formatter_instance, $values) {
  $json = array();
  $json['html'] = feed_item_field_link_format_web($feed_field, $feed_item_field_value, $field_instance, $formatter_instance, $values);
  
  $json['values']['type'] = 'link';
  $json['values']['title'] = check_plain($values['label']);
  $json['values']['url'] = check_url($values['url']);
  $json['attributes'] = feed_object_get_data($feed_item_field_value, FEED_DATA_TYPE_FIELD_ATTRIBUTE, array());
  return $json;
}

/**
 * Feed item field formatter; Formats a text field for rss output.
 */
function feed_item_field_link_format_rss($feed_field, $feed_item_field_value, $field_instance, $formatter_instance, $values) {
  $element = new XmlElement($feed_field->tag, $feed_field->namespace);
  
  if (!empty($values['url']) && is_string($values['url'])) {
    $element->setValue($values['url']);
  }
  $element->setNamespacedAttributes(feed_object_get_data($feed_item_field_value, FEED_DATA_TYPE_FIELD_ATTRIBUTE, array()));
  return $element;
}

/**
 * Feed item field formatter; Formats a link field for text output.
 */
function feed_item_field_link_format_text($feed_field, $feed_item_field_value, $field_instance, $formatter_instance, $values) {
  return url($values['url'], array('absolute' => TRUE));
}

/**
 * Feed item field formatter; Formats a link field for link output.
 */
function feed_item_field_link_format_web($feed_field, $feed_item_field_value, $field_instance, $formatter_instance, $values) {
  if (!empty($values['label'])) {
    return l($values['label'], $values['url'], array(
      'attributes' => array('class' => 'feed-item-field-link'),
      'title' => $values['label'],
      'absolute' => TRUE,
    ));
  }
  else {
    return l($values['url'], $values['url'], array(
      'attributes' => array('class' => 'feed-item-field-link'),
      'title' => $values['url'],
      'absolute' => TRUE,
    ));
  }
}

/**
 * Feed field widget callback; Creates a link feed field widget element
 */
function feed_field_link_widget($feed_field, $instance, $values) {
  
  $form['url'] = array(
    '#type' => 'textfield',
    '#default_value' => !empty($values['url']) ? $values['url'] : '',
    '#title' => !empty($instance['title']) ? $instance['title'] : ucwords($feed_field->name),
    '#description' => !empty($instance['description']) ? $instance['description'] : NULL,
    '#element_validate' => array('feed_field_validate_url'),
    '#required' => !empty($instance['required']),
  );
  
  if ($instance['label']) {
    if (isset($values['label'])) {
      $label = $values['label'];
    }
    else {
      $label = isset($values['url']) ? $values['url'] : '';
    }
    $form['label'] = array(
      '#type' => 'textfield',
      '#default_value' => $instance['maxlength'],
      '#title' => t('Label for URL'),
      '#maxlength' => $instance['maxlength'],
      '#size' => $instance['size'],
      '#required' => TRUE,
    );
  }
  
  return $form;
}

/**
 * Feed field submit callback; Returns data to be stored for widget
 */
function feed_field_link_widget_submit($feed_field, $instance, $values) {
  $data = array();
  if (!empty($values['url'])) {
    $data['url'] = $values['url'];
  }
  if (!empty($values['label'])) {
    $data['label'] = $values['label'];
  }
  return $data;
}

/**
 * Feed field settings callback; Creates a text feed field settings element
 */
function feed_field_link_settings($feed_field, $instance) {
  $form['label'] = array(
    '#type' => 'checkbox',
    '#title' => t('URL label'),
    '#default_value' => $instance['label'],
    '#required' => FALSE,
    '#description' => t('Create label for Link.'),
    '#id' => 'feed-field-link-setting-label',
  );
  
  $form['maxlength'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum length of link label'),
    '#default_value' => $instance['maxlength'],
    '#required' => FALSE,
    '#description' => t('The maximum length of the field in characters.'),
    '#element_validate' => array('feed_field_validate_integer_positive'),
    '#state' => array(
      'visible' => array(
       'input[id="feed-field-link-setting-label"]' => array('checked' => TRUE),
      ),
    ),
  );
  
  $form['size'] = array(
    '#type' => 'textfield',
    '#title' => t('Size of the link label text field'),
    '#default_value' => $instance['size'],
    '#required' => FALSE,
    '#element_validate' => array('feed_field_validate_integer_positive'),
    '#state' => array(
      'visible' => array(
       'input[id="feed-field-link-setting-label"]' => array('checked' => TRUE),
      ),
    ),
  );
  
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Label of the link field'),
    '#default_value' => isset($instance['title']) ? $instance['title'] : t(ucfirst($feed_field->name)),
    '#required' => FALSE,
  );
  
  $form['description'] = array(
    '#type' => 'textfield',
    '#title' => t('Description of the link field'),
    '#default_value' => isset($instance['description']) ? $instance['description'] : t('Link for ' . $feed_field->name),
    '#required' => FALSE,
  );
  return $form;
}
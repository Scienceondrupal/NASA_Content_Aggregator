<?php

/**
 * Feed field formatter; Formats a text field for json output.
 */
function feed_field_date_format_json($feed_field, $feed_field_value, $field_instance, $formatter_instance, $values) {
  $json = array();
  $json['html'] = feed_field_date_format_web($feed_field, $feed_field_value, $field_instance, $formatter_instance, $values);

  $json['values']['type'] = 'date';
  $json['values']['date'] = feed_field_date_create($values['timestamp'], $values['timezone'])->format(DATE_W3C);
  
  $json['attributes'] = feed_object_get_data($feed_field_value, FEED_DATA_TYPE_FIELD_ATTRIBUTE, array());
  return $json;
}

/**
 * Feed field formatter; Formats a date field for rss output.
 */
function feed_field_date_format_rss($feed_field, $feed_field_value, $field_instance, $formatter_instance, $values) {
  $element = new XmlElement($feed_field->tag, $feed_field->namespace);

  if (!empty($values['timestamp'])) {
    $element->setValue(feed_field_date_create($values['timestamp'], $values['timezone'])->format(DATE_RSS));
  }
  $element->setNamespacedAttributes(feed_object_get_data($feed_field_value, FEED_DATA_TYPE_FIELD_ATTRIBUTE, array()));
  return $element;
}

/**
 * Feed field formatter; Formats a date field for text output.
 */
function feed_field_date_format_text($feed_field, $feed_field_value, $field_instance, $formatter_instance, $values) {
  return !empty($field_instance['format']) 
    ? feed_field_date_formatter($field_instance['format'], $values['timestamp'], $values['timezone']) 
    : gmdate(DATE_COOKIE, $values['timestamp']);
}

/**
 * Feed field formatter; Formats a date field for web output.
 */
function feed_field_date_format_web($feed_field, $feed_field_value, $field_instance, $formatter_instance, $values) {
  $date = feed_field_date_format_text($feed_field, $feed_field_value, $field_instance, $formatter_instance, $values);
  return '<div class="feed-item-field-date">' . $date . '</div>';
}

/**
 * Feed field formatter; Formats a date field for json output.
 */
function feed_item_field_date_format_json($feed_field, $feed_item_field_value, $field_instance, $formatter_instance, $values) {
  $json = array();
  $json['html'] = feed_item_field_date_format_web($feed_field, $feed_item_field_value, $field_instance, $formatter_instance, $values);

  $json['values']['type'] = 'date';
  $json['values']['date'] = feed_field_date_create($values['timestamp'], $values['timezone'])->format(DATE_W3C);
  
  $json['attributes'] = feed_object_get_data($feed_item_field_value, FEED_DATA_TYPE_FIELD_ATTRIBUTE, array());
  return $json;
}

/**
 * Feed item field formatter; Formats a date field for rss output.
 */
function feed_item_field_date_format_rss($feed_field, $feed_item_field_value, $field_instance, $formatter_instance, $values) {
  $element = new XmlElement($feed_field->tag, $feed_field->namespace);

  if (!empty($values['timestamp'])) {
    $element->setValue(feed_field_date_create($values['timestamp'], $values['timezone'])->format(DATE_RSS));
  }
  $element->setNamespacedAttributes(feed_object_get_data($feed_item_field_value, FEED_DATA_TYPE_FIELD_ATTRIBUTE, array()));
  return $element;
}

/**
 * Feed item field formatter; Formats a date field for text output.
 */
function feed_item_field_date_format_text($feed_field, $feed_item_field_value, $field_instance, $formatter_instance, $values) {
  return !empty($field_instance['format']) 
    ? feed_field_date_formatter($field_instance['format'], $values['timestamp'], $values['timezone']) 
    : gmdate(DATE_COOKIE, $values['timestamp']);
}

/**
 * Feed item field formatter; Formats a date field for web output.
 */
function feed_item_field_date_format_web($feed_field, $feed_item_field_value, $field_instance, $formatter_instance, $values) {
  $date = feed_item_field_date_format_text($feed_field, $feed_item_field_value, $field_instance, $formatter_instance, $values);
  return '<div class="feed-item-field-date">' . $date . '</div>';
}

/**
 * Feed date formatter.
 */
function feed_field_date_formatter($format, $timestamp, $timezone = NULL) {
  $date = feed_field_date_create($timestamp, $timezone);
  return strtr($format, array(
    '%month' => $date->format('n'),
    '%month-text' => $date->format('F'),
    '%month-text-short' => $date->format('M'),
    '%day' => $date->format('j'),
    '%day-text' => $date->format('l'),
    '%day-text-short' => $date->format('D'),
    '%year' => $date->format('Y'),
    '%year-short' => $date->format('y'),
    '%hour' => $date->format('g'),
    '%minute' => $date->format('i'),
    '%second' => $date->format('s'),
    '%meridiem' => $date->format('A'),
    '%timezone' => $timezone,
  ));
}

/**
 * Feed date instantiation.
 */
function feed_field_date_create($timestamp, $timezone) {
  $date = new DateTime();
  if (!empty($timestamp)) {
    $date->setTimestamp($timestamp);
  }
  if ($timezone) {
    $date->setTimezone(new DateTimeZone($timezone));
  }
  return $date;
}

/**
 * Feed field widget callback; Creates a date feed field widget element
 */
function feed_field_date_widget($feed_field, $instance, $values) {
  $date = new DateTime('now', new DateTimeZone('UTC'));
  
  $form['date'] = array(
    '#type' => 'feed_date',
    '#default_value' => !empty($values['timestamp']) ? $values : array(
      'timestamp' => $date->getTimestamp(),
      'timezone' => 'UTC'
    ),
    '#title' => !empty($instance['title']) ? $instance['title'] : t(ucfirst($feed_field->name)),
    '#required' => !empty($instance['required']),
    '#description' => !empty($instance['description']) ? $instance['description'] : NULL,
  );
  return $form;
}

/**
 * Feed field submit callback; Returns data to be stored for widget
 */
function feed_field_date_widget_submit($feed_field, $instance, $values) {
  $data = array();
  
  if (!empty($values['date']['timestamp'])) {
    $data['timestamp'] = $values['date']['timestamp'];
  }
  if (!empty($values['date']['timezone'])) {
    $data['timezone'] = $values['date']['timezone'];
  }
  return $data;
}

/**
 * Feed field settings callback; Creates a text feed field settings element
 */
function feed_field_date_settings($feed_field, $instance) {
  $markup = '<div class="feed-field-date-format-help">';
    $markup .= 'Use the following tokens to format how the date is displayed.';
    $markup .= '<ul class="feed-field-date-format-help-list">';
      $markup .= '<li>%month: Numeric representation of the month, e.g. 1-12.</li>';
      $markup .= '<li>%month-text: Full textual representation of the month, e.g. January-December.</li>';
      $markup .= '<li>%month-text-short: Short textual representation of a month, e.g. Jan-Dec.</li>';
      $markup .= '<li>%day: Numeric representation of the day of the month, e.g. 1-31.</li>';
      $markup .= '<li>%day-text: Textual representation of the day, e.g. Monday-Sunday.</li>';
      $markup .= '<li>%day-text-short: Short textual representation of the day, e.g. Mon-Sun.</li>';
      $markup .= '<li>%year: Full numerical representation of the year, e.g. 1999-2012.</li>';
      $markup .= '<li>%year-short: Short numerical representation of a year, e.g. 99-12.</li>';
      $markup .= '<li>%hour: Hours ranging from 1-12.</li>';
      $markup .= '<li>%minute: Minutes ranging from 00-60.</li>';
      $markup .= '<li>%second: Seconds ranging from 00-60.</li>';
      $markup .= '<li>%meridiem: The meridiem in AM/PM.</li>';
      $markup .= '<li>%timezone: The selected timezone.</li>';
    $markup .= '</ul>';
  $markup .= '</div>';
  
  $form['format_help'] = array('#markup' => $markup);
  $form['format'] = array(
    '#type' => 'textfield',
    '#title' => t('Format of the date.'),
    '#default_value' => isset($instance['format']) ? $instance['format'] : '',
    '#required' => FALSE,
  );
  
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Label of the date field'),
    '#default_value' => isset($instance['title']) ? $instance['title'] : t(ucfirst($feed_field->name)),
    '#required' => FALSE,
  );
  
  $form['description'] = array(
    '#type' => 'textfield',
    '#title' => t('Description of the date field'),
    '#default_value' => isset($instance['description']) ? $instance['description'] : t('Text value for ' . $feed_field->name),
    '#required' => FALSE,
  );
  return $form;
}
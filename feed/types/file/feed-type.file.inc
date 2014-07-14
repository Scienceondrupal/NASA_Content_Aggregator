<?php

/**
 * Feed field settings callback; Creates a file feed field settings element
 */
function feed_field_file_settings($feed_field, $instance) {
  
  $form['max_filesize'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum upload size'),
    '#default_value' => $instance['max_filesize'],
    '#description' => t('Enter a value like "512" (bytes), "80 KB" (kilobytes) or "50 MB" (megabytes) in order to restrict the allowed file size. If left empty the file sizes will be limited only by PHP\'s maximum post and file upload sizes (current limit <strong>%limit</strong>).', array('%limit' => format_size(file_upload_max_size()))),
    '#size' => 10,
    '#element_validate' => array('feed_field_validate_settings_max_filesize'),
  );
  $form['file_extensions'] = array(
    '#type' => 'textfield',
    '#title' => t('Allowed file extensions'),
    '#default_value' => $extensions,
    '#description' => t('Separate extensions with a space or comma and do not include the leading dot.'),
    '#element_validate' => array('feed_field_validate_settings_extensions'),
    '#required' => TRUE,
  );
  return $form;
}

/**
 * Feed field widget callback; Creates a file feed field widget element
 */
function feed_field_file_widget($feed_field, $instance, $values) {
  $max_filesize = parse_size(file_upload_max_size());
  
  if (!empty($instance['max_filesize']) && parse_size($instance['max_filesize']) < $max_filesize) {
    $max_filesize = parse_size($instance['max_filesize']);
  }
  $form['file'] = array('#tree' => TRUE);
  $form['file']['type'] = array(
    '#type' => 'radios',
    '#options' => array('file' => 'Upload a file', 'url' => 'Link to a file'),
    '#default_value' => isset($values['type']) ? $values['type'] : 'file',
    '#title' => 'Method to select file',
    '#attributes' => array('class' => array('feed-field-widget-file-type')),
  );
  $form['file']['container'] = array(
    '#type' => 'container',
    '#states' => array(
      'visible' => array(
        'input.feed-field-widget-file-type' => array('value' => 'file'),
      ),
    ),
  );
  $form['file']['container']['upload'] = array(
    '#type' => 'managed_file',
    '#extended' => TRUE,
    '#parents' => array('file', 'upload'),
    '#description' => t('Upload a file. File must be less than :size and format must be :extensions.', array(':size' => format_size($max_filesize), ':extensions' => $instance['file_extensions'])),
    '#default_value' => isset($values['file']) ? $values['file'] : '',
    '#upload_location' => feed_field_file_widget_uri($feed_field),
    '#upload_validators' => array(
      'file_validate_size' => array($max_filesize),
      'file_validate_extensions' => array($instance['file_extensions']),
    ),
  );
  $form['file']['url'] = array(
    '#type' => 'textfield',
    '#title' => t('URL'),
    '#default_value' => isset($values['url']) ? $values['url'] : '',
    '#required' => FALSE,
    '#description' => t('An external hyperlink to the file. This is useful when the file is stored elsewhere. Format must be :extensions.', array(':extensions' => $instance['file_extensions'])),
    '#states' => array(
      'visible' => array(
        'input.feed-field-widget-file-type' => array('value' => 'url'),
      ),
    ),
  );
  return $form;
}

/**
 * Feed field submit callback; Returns data to be stored for widget
 */
function feed_field_file_widget_submit($feed_field, $instance, $values) {
  if (!empty($values['file']['upload'])) {
    $values['file'] = $values['file']['upload'];
    $values['type'] = 'file';
  }
  if (!empty($values['file']['url'])) {
    $values['url'] = $values['file']['url'];
    $values['type'] = 'url';
  }
  if (!isset($values['length'])) {
    
  }
  if (!isset($values['type'])) {
    
  }
  return $values;
}

/**
 * Internal function to retrieve file upload location for this feed field
 */
function feed_field_file_widget_uri($feed_field) {
  $path = 'public://feed/file/';
  
  if ($feed = feed_load($feed_field->fid)) {
    return $path . $feed->fid . '/';
  }
  return $path;
}
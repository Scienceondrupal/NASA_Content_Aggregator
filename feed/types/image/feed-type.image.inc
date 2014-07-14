<?php

/**
 * Feed field formatter; Formats an image field for json output.
 */
function feed_field_image_format_json($feed_field, $feed_field_value, $field_instance, $formatter_instance, $values) {
  $json = array();
  $json['html'] = feed_field_image_format_web($feed_field, $feed_field_value, $field_instance, $formatter_instance, $values);
  
  $json['values']['type'] = 'image';
  $json['values']['title'] = check_plain($values['title']);
  $json['values']['url'] = check_url($values['link']);
  $json['values']['description'] = check_plain($values['description']);
  
  if (!empty($values['file'])) {
    $json['values']['src'] = file_create_url($values['file']);
  }
  elseif (!empty($values['url'])) {
    $json['values']['src'] = file_create_url($values['url']);
  }
  $json['attributes'] = feed_object_get_data($feed_field_value, FEED_DATA_TYPE_FIELD_ATTRIBUTE, array());
  return $json;
}

/**
 * Feed field formatter; Formats a text field for rss output.
 */
function feed_field_image_format_rss($feed_field, $feed_field_value, $field_instance, $formatter_instance, $values) {
  $element = new XmlElement('image');
  
  if (!empty($values['title'])) {
    $title = new XmlElement('title');
    $title->setValue($values['title']);
    $element->addChild($title);
  }

  if (!empty($values['link'])) {
    $link = new XmlElement('link');
    $link->setValue($values['link']);
    $element->addChild($link);
  }
  
  if (!empty($values['file'])) {
    $url = new XmlElement('url');
    $url->setValue(file_create_url($values['file']));
    $element->addChild($url);
  }
  elseif (!empty($values['url'])) {
    $url = new XmlElement('link');
    $url->setValue(file_create_url($values['url']));
    $element->addChild($url);
  }

  if (!empty($values['description'])) {
    $description = new XmlElement('description');
    $description->setValue($values['description']);
    $element->addChild($description);
  }
  $element->setNamespacedAttributes(feed_object_get_data($feed_field_value, FEED_DATA_TYPE_FIELD_ATTRIBUTE, array()));
  return $element;
}

/**
 * Feed field formatter; Formats a image field for image output.
 */
function feed_field_image_format_web($feed_field, $feed_field_value, $field_instance, $formatter_instance, $values) {
  $output = '<div class="feed-field-image">';
  
  if (!empty($values['file']) && $file = file_load($values['file'])) {
    $path = file_create_url($file->uri);
  }
  elseif (!empty($values['url'])) {
    $path = $values['url'];
  }
  
  if (!empty($path)) {
    $image = theme('image', array('path' => $path, 'alt' => $values['title'], 'title' => $values['title']));
    $output .= '<div class="feed-field-image-image">' . $image . '</div>';
  }
  
  if (!empty($values['description'])) {
    $output .= '<div class="feed-field-image-description">' . $values['description'] .'</div>';
  }
  $output .= '</div>';
  
  return $output;
}

/**
 * Feed item field formatter; Formats an image field for json output.
 */
function feed_item_field_image_format_json($feed_field, $feed_item_field_value, $field_instance, $formatter_instance, $values) {
  $json = array();
  $json['html'] = feed_item_field_image_format_web($feed_field, $feed_item_field_value, $field_instance, $formatter_instance, $values);
  
  $json['values']['type'] = 'image';
  $json['values']['title'] = check_plain($values['title']);
  $json['values']['url'] = check_url($values['link']);
  $json['values']['description'] = check_plain($values['description']);
  
  if (!empty($values['file'])) {
    $json['values']['src'] = file_create_url($values['file']);
  }
  elseif (!empty($values['url'])) {
    $json['values']['src'] = file_create_url($values['url']);
  }
  $json['attributes'] = feed_object_get_data($feed_item_field_value, FEED_DATA_TYPE_FIELD_ATTRIBUTE, array());
  return $json;
}

/**
 * Feed item field formatter; Formats a text field for rss output.
 */
function feed_item_field_image_format_rss($feed_field, $feed_item_field_value, $field_instance, $formatter_instance, $values) {
  $element = new XmlElement('link');
  
  $attributes = feed_object_get_data($feed_field_value, FEED_DATA_FIELD_TAG_ATTRIBUTE);
  
  if (!empty($attributes) && is_array($attributes)) {
    $element->setAllAttributes($attributes);
  }
  $element->setAttribute('rel', 'enclosure');
  
  if (!empty($values['url'])) {
    if ($extension = pathinfo($values['url'], PATHINFO_EXTENSION) && preg_match('/jpg|jpeg|gif|png/', $extension)) {
      $element->setAttribute('type', 'image/' . strtolower($extension));
    }
    $element->setAttribute('href', file_create_url($values['url']));
  }
  elseif (!empty($values['file'])) {
    if ($extension = pathinfo($values['file'], PATHINFO_EXTENSION) && preg_match('/jpg|jpeg|gif|png/', $extension)) {
      $element->setAttribute('type', 'image/' . strtolower($extension));
    }
    $element->setAttribute('href', file_create_url($values['file']));
  }
  $element->setNamespacedAttributes(feed_object_get_data($feed_item_field_value, FEED_DATA_TYPE_FIELD_ATTRIBUTE, array()));
  return $element;
}

/**
 * Feed field formatter; Formats a image field for image output.
 */
function feed_item_field_image_format_web($feed_field, $feed_item_field_value, $field_instance, $formatter_instance, $values) {
  $output = '<div class="feed-item-field-image">';
  
  if (!empty($values['file'])) {
    $path = $values['file'];
  }
  elseif (!empty($values['url'])) {
    $path = $values['url'];
  }
  
  if (!empty($path)) {
    $image = theme('image', array('path' => $path, 'alt' => $values['title'], 'title' => $values['title']));
    $output .= '<div class="feed-field-image-image">' . $image . '</div>';
  }
  
  if (!empty($values['description'])) {
    $output .= '<div class="feed-item-field-image-description">' . $values['description'] .'</div>';
  }
  $output .= '</div>';
  
  return $output;
}

/**
 * Feed field widget callback; Creates an image feed field widget element
 */
function feed_field_image_widget($feed_field, $instance, $values) {
  $max_filesize = parse_size(file_upload_max_size());
  
  if (!empty($instance['max_filesize']) && parse_size($instance['max_filesize']) < $max_filesize) {
    $max_filesize = parse_size($instance['max_filesize']);
  }
  $form['type'] = array(
    '#type' => 'radios',
    '#options' => array('file' => 'Upload an image file', 'url' => 'Link to an image file'),
    '#default_value' => isset($values['type']) ? $values['type'] : 'file',
    '#title' => 'Method to select image',
    '#attributes' => array('class' => array('feed-field-widget-image-type')),
    '#required' => TRUE,
  );
  $form['upload'] = array(
    '#type' => 'managed_file',
    '#title' => t('Upload'),
    '#description' => t('Upload an image. File must be less than :size and format must be :extensions.', array(':size' => format_size($max_filesize), ':extensions' => 'JPG, PNG, or GIF')),
    '#default_value' => isset($values['file']) ? $values['file'] : '',
    '#upload_location' => feed_field_image_widget_uri($feed_field),
    '#upload_validators' => array(
      'file_validate_size' => array($max_filesize),
      'file_validate_extensions' => array('jpg jpeg png gif'),
    ),
    '#states' => array(
      'visible' => array(
        'input.feed-field-widget-image-type' => array('value' => 'file'),
      ),
    ),
  );
  $form['url'] = array(
    '#type' => 'textfield',
    '#title' => t('URL'),
    '#default_value' => isset($values['url']) ? $values['url'] : '',
    '#required' => FALSE,
    '#description' => t('An external hyperlink to the image. This is useful when the image is stored elsewhere. Format must be :extensions.', array(':extensions' => 'JPG, PNG, or GIF')),
    '#states' => array(
      'visible' => array(
        'input.feed-field-widget-image-type' => array('value' => 'url'),
      ),
    ),
  );
  
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => isset($values['title']) ? $values['title'] : '',
    '#required' => FALSE,
    '#description' => t('The title of the image.'),
  );
  $form['link'] = array(
    '#type' => 'textfield',
    '#title' => t('Link'),
    '#default_value' => isset($values['link']) ? $values['link'] : '',
    '#required' => FALSE,
    '#description' => t('Link this image to the page this feed is related to.'),
  );
  $form['description'] = array(
    '#type' => 'textfield',
    '#title' => t('Description'),
    '#default_value' => isset($values['description']) ? $values['description'] : '',
    '#required' => FALSE,
    '#description' => t('A short description of the image.'),
  );
  return $form;
}

/**
 * Feed field submit callback; Returns data to be stored for widget
 */
function feed_field_image_widget_submit($feed_field, $instance, $values) {
  if (!isset($values['type'])) {
    return FALSE;
  }
  $data = array();
  $data['type'] = $values['type'];
  $data['file'] = !empty($values['upload']) ? $values['upload'] : '';
  $data['url'] = !empty($values['url']) ? $values['url'] : '';
  $data['title'] = !empty($values['title']) ? $values['title'] : '';
  $data['link'] = !empty($values['link']) ? $values['link'] : '';
  $data['description'] = !empty($values['description']) ? $values['description'] : '';
  return $data;
}

/**
 * Feed field settings callback; Creates an image feed field settings element
 */
function feed_field_image_settings($feed_field, $instance) {
  
  $form['default_image'] = array(
    '#title' => t('Default image'),
    '#type' => 'managed_file',
    '#description' => t('If no image is uploaded, this image will be shown on display.'),
    '#default_value' => $instance['default_image'],
    '#upload_location' => 'public://feed/image/default/',
  );
  $form['max_resolution'] = array(
    '#type' => 'item',
    '#title' => t('Maximum image resolution'),
    '#description' => t('The maximum allowed image size expressed as WIDTHxHEIGHT (e.g. 640x480). Leave blank for no restriction. If a larger image is uploaded, it will be resized to reflect the given width and height. Resizing images on upload will cause the loss of <a href="http://en.wikipedia.org/wiki/Exchangeable_image_file_format">EXIF data</a> in the image.'),
    '#element_validate' => array('feed_field_validate_resolution'),
    '#field_prefix' => '<div class="container-inline">',
    '#field_suffix' => '</div>',
    '#tree' => TRUE,
  );
  $form['max_resolution']['width'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum width'),
    '#title_display' => 'invisible',
    '#default_value' => $instance['max_width'],
    '#size' => 5,
    '#maxlength' => 5,
    '#field_suffix' => ' x ',
  );
  $form['max_resolution']['height'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum height'),
    '#title_display' => 'invisible',
    '#default_value' => $instance['max_height'],
    '#size' => 5,
    '#maxlength' => 5,
    '#field_suffix' => ' ' . t('pixels'),
  );
  $form['min_resolution'] = array(
    '#type' => 'item',
    '#title' => t('Minimum image resolution'),
    '#description' => t('The minimum allowed image size expressed as WIDTHxHEIGHT (e.g. 640x480). Leave blank for no restriction. If a larger image is uploaded, it will be resized to reflect the given width and height. Resizing images on upload will cause the loss of <a href="http://en.wikipedia.org/wiki/Exchangeable_image_file_format">EXIF data</a> in the image.'),
    '#element_validate' => array('feed_field_validate_resolution'),
    '#field_prefix' => '<div class="container-inline">',
    '#field_suffix' => '</div>',
    '#tree' => TRUE,
  );
  $form['min_resolution']['width'] = array(
    '#type' => 'textfield',
    '#title' => t('Minimum width'),
    '#title_display' => 'invisible',
    '#default_value' => $instance['min_width'],
    '#size' => 5,
    '#maxlength' => 5,
    '#field_suffix' => ' x ',
  );
  $form['min_resolution']['height'] = array(
    '#type' => 'textfield',
    '#title' => t('Minimum height'),
    '#title_display' => 'invisible',
    '#default_value' => $instance['min_height'],
    '#size' => 5,
    '#maxlength' => 5,
    '#field_suffix' => ' ' . t('pixels'),
  );
  $form['max_filesize'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum upload size'),
    '#default_value' => $instance['max_filesize'],
    '#description' => t('Enter a value like "512" (bytes), "80 KB" (kilobytes) or "50 MB" (megabytes) in order to restrict the allowed file size. If left empty the file sizes will be limited only by PHP\'s maximum post and file upload sizes (current limit <strong>%limit</strong>).', array('%limit' => format_size(file_upload_max_size()))),
    '#size' => 10,
    '#element_validate' => array('feed_field_validate_settings_max_filesize'),
  );
  return $form;
}

/**
 * Feed field submit callback; Returns data to be stored for settings.
 */
function feed_field_image_settings_submit($feed_field, $instance, $values) {
  $values['max_width'] = !empty($values['max_resolution']['max_width']) ? $values['max_resolution']['max_width'] : $instance['max_width'];
  $values['max_height'] = !empty($values['max_resolution']['max_width']) ? $values['max_resolution']['max_width'] : $instance['max_height'];
  $values['min_width'] = !empty($values['min_resolution']['min_width']) ? $values['min_resolution']['min_width'] : $instance['min_width'];
  $values['min_height'] = !empty($values['min_resolution']['min_width']) ? $values['min_resolution']['min_width'] : $instance['min_height'];
  
  return $values;
}

/**
 * Internal function to retrieve image upload location for this feed field
 */
function feed_field_image_widget_uri($feed_field) {
  $path = 'public://feed/image/';
  
  if (isset($feed_field->fid) && $feed = feed_load($feed_field->fid)) {
    return $path . $feed->fid . '/';
  }
  return $path;
}
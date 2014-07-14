<?php

/**
 * Implements hook_feed_field_type_info().
 */
function feed_feed_field_type_info() {
  return array(
    'text' => array(
      'label' => t('Text'),
      'description' => t('This field displays text in the feed tags'),
      'settings' => array('maxlength' => 255, 'size' => 60, 'default_value' => '', 'title' => '', 'description' => ''),
      'defaults' => array('text' => ''),
      'file' => 'types/text/feed-type.text.inc',
      'feed_field' => TRUE,
      'feed_item_field' => TRUE,
      'form_widget' => 'feed_field_text_widget',
      'form_settings' => 'feed_field_text_settings',
      'submit_widget' => 'feed_field_text_widget_submit',
      'feed_field_formatters' => array(
        'web' => 'feed_field_text_format_web',
        'rss' => 'feed_field_text_format_rss',
        'json' => 'feed_field_text_format_json',
        'text' => 'feed_field_text_format_text',
      ),
      'feed_item_field_formatters' => array(
        'web' => 'feed_item_field_text_format_web',
        'rss' => 'feed_item_field_text_format_rss',
        'json' => 'feed_item_field_text_format_json',
        'text' => 'feed_item_field_text_format_text',
      ),
    ),
    'image' => array(
      'label' => t('Image'),
      'description' => t('This field displays an image'),
      'settings' => array('default_image' => '', 'max_height' => '', 'max_width' => '', 'min_height' => '', 'min_width' => '', 'max_filesize' => '', 'title' => '', 'description' => ''),
      'defaults' => array('file' => '', 'url' => '', 'type' => '', 'title' => '', 'link' => '', 'description' => ''),
      'file' => 'types/image/feed-type.image.inc',
      'feed_field' => TRUE,
      'feed_item_field' => FALSE,
      'form_widget' => 'feed_field_image_widget',
      'form_settings' => 'feed_field_image_settings',
      'submit_widget' => 'feed_field_image_widget_submit',
      'feed_field_formatters' => array(
        'web' => 'feed_field_image_format_web',
        'rss' => 'feed_field_image_format_rss',
        'json' => 'feed_field_image_format_json',
      ),
      'feed_item_field_formatters' => array(
        'web' => 'feed_item_field_image_format_web',
        'rss' => 'feed_item_field_image_format_rss',
        'json' => 'feed_item_field_image_format_json',
      ),
    ),
    'file' => array(
      'label' => t('File'),
      'description' => t('This field displays a file'),
      'settings' => array('max_filesize' => '', 'file_extensions' => '', 'title' => '', 'description' => ''),
      'defaults' => array('file' => '', 'url' => '', 'type' => '', 'mime' => '', 'length' => ''),
      'file' => 'types/file/feed-type.file.inc',
      'feed_field' => FALSE,
      'feed_item_field' => TRUE,
      'form_widget' => 'feed_field_file_widget',
      'form_settings' => 'feed_field_file_settings',
      'submit_widget' => 'feed_field_file_widget_submit',
      'feed_field_formatters' => array(
        'web' => 'feed_field_file_format_web',
        'rss' => 'feed_field_file_format_rss',
        'json' => 'feed_field_file_format_json',
      ),
      'feed_item_field_formatters' => array(
        'web' => 'feed_item_field_file_format_web',
        'rss' => 'feed_item_field_file_format_rss',
        'json' => 'feed_item_field_file_format_json',
      ),
    ),
    'link' => array(
      'label' => t('Link'),
      'description' => t('This field displays a link'),
      'settings' => array('label' => FALSE, 'maxlength' => 100, 'size' => 60, 'title' => '', 'description' => ''),
      'defaults' => array('url' => '', 'label' => ''),
      'file' => 'types/link/feed-type.link.inc',
      'feed_field' => TRUE,
      'feed_item_field' => TRUE,
      'form_widget' => 'feed_field_link_widget',
      'form_settings' => 'feed_field_link_settings',
      'submit_widget' => 'feed_field_link_widget_submit',
      'feed_field_formatters' => array(
        'web' => 'feed_field_link_format_web',
        'rss' => 'feed_field_link_format_rss',
        'json' => 'feed_field_link_format_json',
        'text' => 'feed_field_link_format_text',
      ),
      'feed_item_field_formatters' => array(
        'web' => 'feed_item_field_link_format_web',
        'rss' => 'feed_item_field_link_format_rss',
        'json' => 'feed_item_field_link_format_json',
        'text' => 'feed_item_field_link_format_text',
      ),
    ),
    'html' => array(
      'label' => t('HTML'),
      'description' => t('This field displays formatted HTML in the feed tags'),
      'settings' => array(
      	'size' => 60, 'default_value' => '', 'title' => '', 'description' => '',
      	'allowed' => 'div span p h1 h2 h3 h4 h5 a em strong cite blockquote code ul ol li dl dt dd'
      ),
      'defaults' => array('html' => '', 'text' => '', 'format' => ''),
      'file' => 'types/html/feed-type.html.inc',
      'feed_field' => TRUE,
      'feed_item_field' => TRUE,
      'form_widget' => 'feed_field_html_widget',
      'form_settings' => 'feed_field_html_settings',
      'submit_widget' => 'feed_field_html_widget_submit',
      'feed_field_formatters' => array(
        'web' => 'feed_field_html_format_web',
        'rss' => 'feed_field_html_format_rss',
        'json' => 'feed_field_html_format_json',
        'text' => 'feed_field_html_format_text',
      ),
      'feed_item_field_formatters' => array(
        'web' => 'feed_item_field_html_format_web',
        'rss' => 'feed_item_field_html_format_rss',
        'json' => 'feed_item_field_html_format_json',
        'text' => 'feed_item_field_html_format_text',
      ),
    ),
    'date' => array(
      'label' => t('Date'),
      'description' => t('This field displays formatted date in the feed tags'),
      'settings' => array(
      	'format' => '%month-%day-%year %hour:%minute %meridiem', 'required' => FALSE, 'title' => '', 'description' => '',
      ),
      'defaults' => array('timestamp' => '', 'timezone' => ''),
      'file' => 'types/date/feed-type.date.inc',
      'feed_field' => TRUE,
      'feed_item_field' => TRUE,
      'form_widget' => 'feed_field_date_widget',
      'form_settings' => 'feed_field_date_settings',
      'submit_widget' => 'feed_field_date_widget_submit',
      'feed_field_formatters' => array(
        'web' => 'feed_field_date_format_web',
        'rss' => 'feed_field_date_format_rss',
        'json' => 'feed_field_date_format_json',
        'text' => 'feed_field_date_format_text',
      ),
      'feed_item_field_formatters' => array(
        'web' => 'feed_item_field_date_format_web',
        'rss' => 'feed_item_field_date_format_rss',
        'json' => 'feed_item_field_date_format_json',
        'text' => 'feed_item_field_date_format_text',
      ),
    ),
  );
}

/**
 * Retrieve field type option list
 * 
 * @param $list
 *   An array of available feed field type names to use in option.
 */
function feed_field_type_options(array $list = array()) {
  $options = &drupal_static(__FUNCTION__);
  
  if (!isset($options)) {
    $options = array();
    
    if ($infos = feed_field_type_info()) {
      if (empty($list)) {
        $list = array_keys($infos);
      }
      foreach ($list as $name) {
        if (isset($infos[$name])) {
          $options[$name] = isset($infos[$name]['label']) ? $infos[$name]['label'] : $name;
        }
      }
    }
  }
  return $options;
}

/**
 * Retrieve info about an field type
 * 
 * @param $type
 *   A field type name, this is the array key that modules implementing
 *   hook_feed_field_type_info() will return.
 * @param $load
 *   Boolean to indicate whether to load file or not if a type is specified
 */
function feed_field_type_info($type = NULL, $load = FALSE) {
  $feed_field_types = &drupal_static(__FUNCTION__);
  
  if (!isset($feed_field_types)) {
    if (false && $cache = cache_get('feed_field_type_info', 'cache')) {
      $feed_field_types = $cache->data;
    }
    else {
      foreach (module_implements('feed_field_type_info') as $module) {
        foreach (module_invoke($module, 'feed_field_type_info') as $name => $info) {
          $info['module'] = $module;
          
          $feed_field_types[$name] = $info;
        }
      }
      drupal_alter('feed_field_type_info', $feed_field_types);
      cache_set('feed_field_type_info', $feed_field_types);
    }
  }
  
  if (!empty($type)) {
    if (isset($feed_field_types[$type])) {
      if (!empty($load) && isset($feed_field_types[$type]['file'])) {
        $path = drupal_get_path('module', $feed_field_types[$type]['module']);
        $file = $path . DIRECTORY_SEPARATOR . $feed_field_types[$type]['file'];
        
        if (file_exists($file)) {
          include_once($file);
        }
        else {
          watchdog('feed', 'File was not able to be included for feed field type :type', array(':type' => $type));
        }
      }
      return $feed_field_types[$type];
    }
    return FALSE;
  }
  else {
    return $feed_field_types;
  }
}

/**
 * Validator callback; Validates an image resolution.
 */
function feed_field_validate_resolution($element, &$form_state) {
  if (!empty($element['width']['#value']) || !empty($element['height']['#value'])) {
    foreach (array('width', 'height') as $dimension) {
      $value = $element[$dimension]['#value'];
      if (!is_numeric($value)) {
        form_error($element[$dimension], t('Height and width values must be numeric.'));
        return;
      }
      if (intval($value) == 0) {
        form_error($element[$dimension], t('Both a height and width value must be specified in the !name field.', array('!name' => $element['#title'])));
        return;
      }
    }
    form_set_value($element, intval($element['width']['#value']) . 'x' . intval($element['height']['#value']), $form_state);
  }
  else {
    form_set_value($element, '', $form_state);
  }
}

/**
 * Validator callback; Validates an url.
 */
function feed_field_validate_url($element, &$form_state) {
  if (!empty($element['#value']) && !valid_url($element['#value'], TRUE)) {
    form_error($element, t('The !name field must be a fully qualified and valid URL (i.e. including http:// or https:// schemes)', array('!name' => $element['#title'])));
  }
}

/**
 * Validator callback; Validates the settings file extensions field.
 *
 * This doubles as a convenience clean-up function and a validation routine.
 * Commas are allowed by the end-user, but ultimately the value will be stored
 * as a space-separated list for compatibility with file_validate_extensions().
 */
function feed_field_validate_settings_extensions($element, &$form_state) {
  if (!empty($element['#value'])) {
    $extensions = preg_replace('/([, ]+\.?)/', ' ', trim(strtolower($element['#value'])));
    $extensions = array_filter(explode(' ', $extensions));
    $extensions = implode(' ', array_unique($extensions));
    if (!preg_match('/^([a-z0-9]+([.][a-z0-9])* ?)+$/', $extensions)) {
      form_error($element, t('The list of allowed extensions is not valid, be sure to exclude leading dots and to separate extensions with a comma or space.'));
    }
    else {
      form_set_value($element, $extensions, $form_state);
    }
  }
}

/**
 * Validator callback; Validates an image file size.
 */
function feed_field_validate_settings_max_filesize($element, &$form_state) {
  if (!empty($element['#value']) && !is_numeric(parse_size($element['#value']))) {
    form_error($element, t('The "!name" option must contain a valid value. You may either leave the text field empty or enter a string like "512" (bytes), "80 KB" (kilobytes) or "50 MB" (megabytes).', array('!name' => t($element['title']))));
  }
}
  
/**
 * Validator callback; Validates an integer is positive.
 */
function feed_field_validate_integer_positive($element, &$form_state) {
  $value = $element['#value'];
  if ($value !== '' && (!is_numeric($value) || intval($value) != $value || $value <= 0)) {
    form_error($element, t('%name must be a positive integer.', array('%name' => $element['#title'])));
  }
}
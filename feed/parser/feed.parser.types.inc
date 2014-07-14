<?php

/**
 * Retrieves parser structure type infos
 */
function feed_parser_type_info($parser, $type = NULL, $load = FALSE) {
  $types = &drupal_static(__FUNCTION__, array());
  
  if (!isset($types[$parser])) {
    $hook = 'feed_' . $parser . '_parser_types';
    
    if (false && $cache = cache_get($hook, 'cache')) {
      $types[$parser] = $cache->data;
    }
    else {
      $types[$parser] = array();
      
      foreach (module_implements($hook) as $module) {
        foreach (module_invoke($module, $hook) as $name => $info) {
          $types[$parser][$name] = $info;
          $types[$parser][$name]['module'] = $module;
          $types[$parser][$name]['name'] = $name;
        }
      }
      cache_set($hook, $types[$parser]);
    }
  }
  
  if ($type === NULL) {
    return $types[$parser];
  }
  elseif (isset($types[$parser][$type])) {
    $info = $types[$parser][$type];
    if ($load && isset($info['module']) && !isset($types[$parser][$type]['loaded'])) {
      $module = $info['module'];
      
      if (isset($info['file'])) {
        $file = drupal_get_path('module', $module) . DIRECTORY_SEPARATOR . $info['file'];
        
        if (file_exists($file)) {
          include_once $file;
        }
      }
      else {
        module_load_include('inc', $module);
      }
      $types[$parser][$type]['loaded'] = TRUE;
    }
    return $info;
  }
  return FALSE;
}

/**
 * XML structure validator;
 * 
 * Example config:
 *   array(
 *     XmlElement::KEY_TAG => {Tag},
 *     XmlElement::KEY_ATTRIBUTE => array(
 *       {Attribute Name} => {Attribute Value},
 *     ),
 *     XmlElement::KEY_CHILDREN => array(
 *       {Child Structure 1},
 *       {Child Structure 2},
 *     ),
 *   ),
 */
function feed_parser_validate_structure(array $elements = array(), array $structures = array(), &$ref = array()) {
  foreach ($structures as $structure_name => $structure) {
    if (feed_parser_validate_structure_compare($structure, $elements)) {
      if (!empty($structure[XmlElement::KEY_CHILDREN])) {
        foreach ($elements[XmlElement::KEY_CHILDREN] as $child_element) {
          feed_parser_validate_structure($child_element, $structure[XmlElement::KEY_CHILDREN], $ref);
        }
      }
      $ref[$structure_name] = $elements;
    }
  }
}

/**
 * Recursively compares structure1 against structure2 and returns false if all
 * elements in structure1 is not present in structure2.
 * 
 * @param $structure1
 * @param $structure2
 */
function feed_parser_validate_structure_compare(array $structure1, array $structure2) {
  if (count(array_diff(array_filter($structure1, 'is_scalar'), $structure2))) {
    return FALSE;
  }
  
  foreach (array_filter($structure1, 'is_array') as $key => $struct1) {
    if (!isset($structure2[$key]) || !is_array($structure2[$key])) {
      return FALSE;
    }
    if (!feed_parser_validate_structure_compare($struct1, $structure2[$key])) {
      return FALSE;
    }
  }
  return TRUE;
}

/**
 * Process parser type object callbacks
 */
function feed_parser_type_process_object($parser, $type, $object, $arguments = array()) {
  if (!$info = feed_parser_type_info($parser, $type, TRUE)) {
    watchdog('feed', 'Cannot parse feed item field %field, parser field type %type for parser %parser could not be found', array('%type' => $type, '%parser' => $parser));
    return FALSE;
  }
  
  $values = array();
  if (!empty($info['objects'][$object]) && is_array($info['objects'][$object])) {
    foreach ($info['objects'][$object] as $data => $callbacks) {
      foreach ($callbacks as $callback => $options) {
        if (is_callable($callback)) {
          $values[$data] = call_user_func_array($callback, array($options, $arguments));
        }
      }
    }
  }
  return $values;
}

/**
 * XML get element tag from a structure;
 */
function feed_parser_structure_tag($options, $arguments) {
  if (!is_array($options)) {
    $options = array($options);
  }
  
  if (is_array($arguments)) {
    foreach ($options as $name) {
      if (isset($arguments[$name][XmlElement::KEY_TAG_NAME])) {
        return trim($arguments[$name][XmlElement::KEY_TAG_NAME]);
      }
    }
  }
  return NULL;
}

/**
 * XML get element namespace from a structure;
 */
function feed_parser_structure_tag_namespace($options, $arguments) {
  if (!is_array($options)) {
    $options = array($options);
  }
  
  if (is_array($arguments)) {
    foreach ($options as $name) {
      if (isset($arguments[$name][XmlElement::KEY_TAG_NAMESPACE])) {
        return trim($arguments[$name][XmlElement::KEY_TAG_NAMESPACE]);
      }
    }
  }
  return NULL;
}

/**
 * XML get element value from a structure;
 */
function feed_parser_structure_value($options, $arguments) {
  if (!is_array($options)) {
    $options = array($options);
  }
  
  if (is_array($arguments)) {
    foreach ($options as $name) {
      if (isset($arguments[$name][XmlElement::KEY_VALUE])) {
        return trim($arguments[$name][XmlElement::KEY_VALUE]);
      }
    }
  }
  return NULL;
}

/**
 * XML get element attribute from a structure;
 */
function feed_parser_structure_attribute($options, $arguments) {
  if (is_array($options['names']) && !empty($options['attribute']) && is_array($arguments)) {
    foreach ($options['names'] as $name) {
      if (isset($arguments[$name][XmlElement::KEY_ATTRIBUTE][$options['attribute']])) {
        return trim($arguments[$name][XmlElement::KEY_ATTRIBUTE][$options['attribute']]);
      }
    }
  }
  return NULL;
}
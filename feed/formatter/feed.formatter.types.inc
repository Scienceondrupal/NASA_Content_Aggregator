<?php

/**
 * Retrieves formatter type infos
 */
function feed_formatter_type_infos($formatter) {
  $formatter_types = &drupal_static(__FUNCTION__, array());
  
  if (!isset($formatter_types[$formatter])) {
    $hook = 'feed_' . $formatter . '_formatter_types';
    
    if (false && $cache = cache_get($hook, 'cache')) {
      $formatter_types[$formatter] = $cache->data;
    }
    else {
      $formatter_types[$formatter] = array();
      
      foreach (module_implements($hook) as $module) {
        foreach (module_invoke($module, $hook) as $name => $object_infos) {
          foreach ($object_infos as $object => &$infos) {
            $infos['module'] = $module;
            $infos['name'] = $name;
          }
          $formatter_types[$formatter][$name] = $object_infos;
        }
      }
      cache_set($hook, $formatter_types[$formatter]);
    }
  }
  
  return $formatter_types[$formatter];
}

/**
 * Retrieves formatter type info by name
 */
function feed_formatter_type_info_by_name($formatter, $name) {
  $formatter_types = &drupal_static(__FUNCTION__, array());
  
  if (!isset($formatter_types[$formatter])) {
    $formatter_types[$formatter] = feed_formatter_type_infos($formatter);
  }
  return isset($formatter_types[$formatter][$name]) ? $formatter_types[$formatter][$name] : array();
}

/**
 * Retrieves formatter type info by name
 */
function feed_formatter_type_info_by_hook($formatter, $hook) {
  $formatter_types = &drupal_static(__FUNCTION__, array());
  
  if (!isset($formatter_types[$formatter])) {
    $formatter_types[$formatter] = array();
  }
  
  if (!isset($formatter_types[$formatter][$hook])) {
    $formatter_types[$formatter][$hook] = array();
    
    $formatter_type_infos = feed_formatter_type_infos($formatter);
    
    foreach ($formatter_type_infos as $name => $hooks) {
      if (isset($hooks[$hook])) {
        $formatter_types[$formatter][$hook][$name] = $hooks[$hook];
      }
    }
  }
  return $formatter_types[$formatter][$hook];
}

/**
 * Validates and process all callbacks for a formatter hook.
 * 
 * @param $formatter
 *   The formatter to process.
 * @param $hook
 *   The hook to process.
 * @param $variables
 *   The variables to process in callbacks.
 */
function feed_formatter_type_process($formatter, $hook, &$variables) {
  $variables['#values'] = array();
  
  foreach (feed_formatter_type_info_by_hook($formatter, $hook) as $name => $formatter_type) {
    if (feed_formatter_type_validate($formatter_type, $variables)) {
      $variables['#values'] = feed_formatter_type_format($formatter_type, $variables);
  
      if (!empty($variables['#values']) && is_array($variables['#values'])) {
        $variables['#values'] = feed_formatter_type_filter($formatter_type, $variables['#values']);
      }
      
      if (!empty($formatter_type['theme'])) {
        $variables['#content'] = theme($formatter_type['theme'], $variables['#values']);
      }
      return TRUE;
    }
  }
  return FALSE;
}

/**
 * Main Feed Formatter Theme Method; Runs the theming callback.
 * 
 * @param $formatter_type
 *   A formatter type info array.
 * @param $values
 *   Values to filter.
 */
function feed_formatter_type_theme(array $formatter_type, array $values) {
  if (!empty($formatter_type['theme'])) {
    return theme($formatter_type['theme'], $values);
  }
  return $filtered;
}

/**
 * Main Feed Formatter Filter Method; Runs the filtering callbacks on a set of 
 * variables for a field type.
 * 
 * @param $formatter_type
 *   A formatter type info array.
 * @param $values
 *   Values to filter.
 */
function feed_formatter_type_filter(array $formatter_type, array $values) {
  $filtered = $values;
  
  if (!empty($formatter_type['filters'])) {
    foreach ($formatter_type['filters'] as $key => $method) {
      if (isset($values[$key]) && is_callable($method)) {
        $filtered[$key] = call_user_func($method, array($values[$key]));
      }
    }
  }
  return $filtered;
}

/**
 * Main Feed Formatter Format Method; Runs the formatting callbacks on a set of 
 * variables for a field type.
 * 
 * @param $formatter_type
 *   A formatter type info array.
 * @param $values
 *   Variables to pass to format callback.
 */
function feed_formatter_type_format(array $formatter_type, array $variables) {
  $formatted = array();
  
  if (!empty($formatter_type['formatters'])) {
    foreach ($formatter_type['formatters'] as $object => $values) {
      if (!is_array($values) || !isset($variables[$object])) {
        continue;
      }
      
      foreach ($values as $key => $callbacks) {
        if (!is_array($callbacks)) {
          continue;
        }
        
        foreach ($callbacks as $callback => $arguments) {
          if (is_string($arguments)) {
            $callback = $arguments;
            $arguments = array();
          }
          array_unshift($arguments, $variables[$object]);
          
          if (is_callable($callback)) {
            $formatted[$key] = call_user_func_array($callback, $arguments);
          }
        }
      }
    }
  }
  return $formatted;
}

/**
 * Main Feed Formatter Validator Method; Runs the validation callbacks on a set 
 * of variables for a field type.
 * 
 * @param $formatter_type
 *   A formatter type info array.
 * @param $variables
 *   Variables to pass to validate callback.
 */
function feed_formatter_type_validate($formatter_type, array $variables) {
  if (!isset($formatter_type['validators'])) {
    return TRUE;
  }
  
  if (is_array($formatter_type['validators'])) {
    foreach ($formatter_type['validators'] as $object => $callbacks) {
      if (!isset($variables[$object]) || !is_array($callbacks)) {
        return FALSE;
      }
      
      foreach ($callbacks as $callback => $args) {
        $method = is_string($args) ? $args : $callback;
        $arguments = array(&$variables[$object]);
        
        if (!is_callable($method) || !call_user_func_array($method, array_merge($arguments, (array) $args))) {
          return FALSE;
        }
      }
    }
    return TRUE;
  }
  
  return FALSE;
}

/**
 * Formatter Type Validator Callback; Checks an object for a property.
 */
function feed_formatter_type_validate_object_properties($object, array $properties) {
  if (!is_object($object)) {
    return FALSE;
  }
  
  foreach ($properties as $name => $property) {
    if (!isset($object->{$name}) || $object->{$name} != $property) {
      return FALSE;
    }
  }
  return TRUE;
}

/**
 * Formatter Type Validator Callback; Checks an object for a data value.
 */
function feed_formatter_type_validate_object_data($object, array $datas) {
  if (!is_object($object) || !is_array($object->data)) {
    return FALSE;
  }
  
  foreach ($datas as $data => $value) {
    if (feed_object_get_data($object, $data) != $value) {
      return FALSE;
    }
  }
  return TRUE;
}
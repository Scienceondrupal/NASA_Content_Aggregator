<?php

/**
 * Implements hook_feed_parsers().
 */
function feed_feed_parsers() {
  $parsers['rss'] = array(
    'label' => 'RSS',
    'file' => 'parser/rss/feed-parser.rss.inc',
  
    'retrieve_feed_field_elements' => 'feed_parser_rss_retrieve_feed_field_elements',
    'retrieve_feed_item_elements' => 'feed_parser_rss_retrieve_feed_item_elements',
    'retrieve_feed_item_field_elements' => 'feed_parser_rss_retrieve_feed_item_field_elements',
  
    'load_feed_item' => 'feed_parser_rss_load_feed_item',
    'load_feed' => 'feed_parser_rss_load_feed',
  
    'config_field' => 'feed_parser_rss_config_field',

    'parse_feed_field' => 'feed_parser_rss_parse_feed_field',
    'parse_feed_field_value' => 'feed_parser_rss_parse_feed_field_value',
    'parse_feed_item_field' => 'feed_parser_rss_parse_feed_item_field',
    'parse_feed_item_field_value' => 'feed_parser_rss_parse_feed_item_field_value',
  
    'type_feed_field' => 'feed_parser_rss_feed_field_type',
    'type_feed_item_field' => 'feed_parser_rss_feed_item_field_type',
  );
  return $parsers;
}

/**
 * Retrieves parser infos
 */
function feed_parser_info($parser = NULL, $load = FALSE) {
  $feed_parser_infos = &drupal_static(__FUNCTION__);
  
  if (!isset($feed_parser_infos)) {
    if (false && $cache = cache_get('feed_parser_info', 'cache')) {
      $feed_parser_infos = $cache->data;
    }
    else {
      $feed_parser_infos = array();
      foreach (module_implements('feed_parsers') as $module) {
        foreach (module_invoke($module, 'feed_parsers') as $name => $info) {
          $feed_parser_infos[$name] = $info;
          $feed_parser_infos[$name]['module'] = $module;
          $feed_parser_infos[$name]['name'] = $name;
        }
      }
      cache_set('feed_parser_info', $feed_parser_infos);
    }
  }
  
  if ($parser === NULL) {
    return $feed_parser_infos;
  }
  elseif (isset($feed_parser_infos[$parser])) {
    $info = $feed_parser_infos[$parser];
    if ($load && isset($info['module']) && !isset($feed_parser_infos[$parser]['loaded'])) {
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
      $feed_parser_infos[$parser]['loaded'] = TRUE;
    }
    return $info;
  }
  return FALSE;
}

function feed_parser_info_options() {
  $feed_parser_infos = feed_parser_info();
  
  $options = array();
  foreach ($feed_parser_infos as $name => $feed_parser_info) {
    $options[$name] = isset($feed_parser_info['label']) ? $feed_parser_info['label'] : NULL;
  }
  return $options;
}

/**
 * Parser method invoker; This function invokes the parser methods
 * 
 * @param $type
 *   Parser type as specified by the key in hook_feed_parsers implementations.
 * @param $method
 *   Method to call parser function, can be a string or an array.
 * @param ...$arguments
 *   Every additional parameters will be passed as an argument to the method
 */
function feed_parser_invoke($type, $method) {
  $args = func_get_args();
  $type = array_shift($args);
  $method = array_shift($args);
  $return = FALSE;
  
  if ($info = feed_parser_info($type, TRUE)) {
    if (isset($info[$method])) {
      try {
        $return = call_user_func_array($info[$method], $args);
      }
      catch (Exception $e) {
        watchdog_exception('feed', $e);
      }
    }
  }
  else {
    watchdog('feed', 'Could not find feed parser for type :type', array(':type' => $type));
  }
  return $return;
}

/**
 * Returns the configuration for feed from a root XmlElement object.
 */
function feed_config_parse($type, XmlElements $elements) {
  $configs = array();
  
  $feed_field_elements = feed_parser_retrieve_feed_field_elements($type, $elements);
  foreach ($feed_field_elements as $feed_field_element) {
    if ($feed_field_element instanceof XmlElement) {
      $config = feed_parser_invoke($type, 'config_field', $feed_field_element);
      
      // Use namespace and tagname as identifier
      $tag_name = $feed_field_element->getTag();
      $tag_namespace = $feed_field_element->getTagNamespace();
      
      $configs['feed_fields']["{$tag_namespace}_{$tag_name}"] = $config + array(
        'name' => !empty($tag_namespace) ? "{$tag_name} {$tag_name}" : $tag_name,
        'tag' => !empty($tag_namespace) ? "{$tag_namespace}:{$tag_name}" : $tag_name,
      );
    }
  }
  
  $feed_item_elements = feed_parser_retrieve_feed_item_elements($type, $elements);
  if ($feed_item_elements instanceof XmlElements && count($feed_item_elements)) {
    // Initialize the feed_item_fields config array.
    $configs['feed_item_fields'] = array();
    
    foreach ($feed_item_elements->reverse() as $feed_item_index => $feed_item_element) {
      $configs['feed_item_fields'][$feed_item_index] = array();
      
      if ($feed_item_element instanceof XmlElement) {
        $feed_item_field_elements = feed_parser_retrieve_feed_item_field_elements($type, $feed_item_element);
      
        if (!empty($feed_item_field_elements) && count($feed_item_field_elements)) {
          foreach ($feed_item_field_elements as $feed_item_field_element) {
            if ($feed_item_field_element instanceof XmlElement) {
              $config = feed_parser_invoke($type, 'config_field', $feed_item_field_element);
              
              // Use namespace and tagname as identifier
              $tag_name = $feed_item_field_element->getTag();
              $tag_namespace = $feed_item_field_element->getTagNamespace();
              
              $configs['feed_item_fields'][$feed_item_index]["{$tag_namespace}_{$tag_name}"] = $config + array(
                'name' => !empty($tag_namespace) ? "{$tag_name} {$tag_name}" : $tag_name,
                'tag' => !empty($tag_namespace) ? "{$tag_namespace}:{$tag_name}" : $tag_name,
              );
            }
          }
        }
      }
    }
  }
  return $configs;
}

/**
 * Parse a feed from a root XmlElement object.
 */
function feed_parse($objects = array(), $type, XmlElements $elements, $configs) {
  if (!isset($objects['feed'])) {
    return FALSE;
  }
  
  $feed_items = array();
  $feed_fields = array();
  $feed_item_fields = array();
  
  // Try to load the feed object from element
  $feed = feed_parser_load_feed($type, $elements, isset($objects['feed']) ? $objects['feed'] : NULL);
  
  if (isset($configs['feed_fields'])) {
    $feed->config = $configs['feed_fields'];
  }
  
  $feed_field_elements = feed_parser_retrieve_feed_field_elements($type, $elements);
  if (!empty($feed_field_elements) && count($feed_field_elements)) {
    $feed_field_indexes = array();
    
    // Load feed_field once into the feed_fields object array.
    if (isset($feed->fid)) {
      foreach (feed_load_fields($feed, FALSE) as $feed_field) {
        $identifier = sprintf('%s_%s', $feed_field->namespace, $feed_field->tag);
        $feed_fields[$identifier] = $feed_field;
      }
    }
    
    foreach ($feed_field_elements as $feed_field_element) {
      if ($feed_field_element instanceof XmlElement) {
        // Use namespace and tagname as identifier
        $identifier = sprintf('%s_%s', $feed_field_element->getTagNamespace(), $feed_field_element->getTag());
        
        // Keep track of indexes of feed_field_values
        if (!isset($feed_field_indexes[$identifier])) {
          $feed_field_indexes[$identifier] = 0;
        }
        else {
          $feed_field_indexes[$identifier] += 1;
        }
        
        // Get the index of the feed field value we will be creating/modifying
        $index = $feed_field_indexes[$identifier];
        
        // Load feed_field_values once into feed_field_values object array
        if (!isset($feed_field_values[$identifier])) {
          $feed_field_values[$identifier] = array();
        
          // Only load if feed field is existing
          if (isset($feed->fid, $feed_fields[$identifier]->ffid)) {
            foreach (feed_field_values_load($feed, $feed_fields[$identifier], TRUE) as $feed_field_value) {
              $feed_field_values[$identifier][] = $feed_field_value;
            }
          }
        }
        // Initialize the feed field object if it does not exists.
        if (!isset($feed_fields[$identifier])) {
          if (isset($objects['feed_fields'][$identifier])) {
            $feed_fields[$identifier] = $objects['feed_fields'][$identifier];
          }
          else {
            $feed_fields[$identifier] = feed_field_defaults();
          }
        }
        
        // Initialize the feed field value object if it does not exists.
        if (!isset($feed_field_values[$identifier][$index])) {
          if (isset($objects['feed_field_values'][$identifier][$index])) {
            $feed_field_values[$identifier][$index] = $objects['feed_field_values'][$identifier][$index];
          }
          else {
            $feed_field_values[$identifier][$index] = feed_field_value_defaults();
          }
          feed_object_set_data($feed_field_values[$identifier][$index], FEED_DATA_TYPE_FIELD_ATTRIBUTE, $feed_field_element->getNamespacedAttributes());
        }
        
        // Create the reference objects so we can modify these objects in invoker
        $reference = array();
        $reference['#feed'] =& $feed;
        $reference['#feed_field'] =& $feed_fields[$identifier];
        $reference['#feed_field_value'] =& $feed_field_values[$identifier][$index];
        
        // Pull the configurations from the config array for this field
        $config = array();
        if (isset($feed->config[$identifier])) {
          $config = $feed->config[$identifier];
          
          if ($structure = feed_object_get_data($feed_fields[$identifier], FEED_DATA_FEED_FIELD_STRUCTURE)) {
            $config['default'] = $structure;
          }
          elseif (!isset($config['default']) && !empty($config['types'])) {
            $config['default'] = reset(array_keys($config['types']));
          }
        }
        feed_parser_parse_feed_field($type, $feed_field_element, $reference, $config);
        
        // Remove feed fields if the reference does not exists anymore.
        if (empty($reference['#feed_field'])) {
          unset($feed_fields[$identifier]);
        }
        
        // Remove feed field values if the reference does not exists anymore.
        if (empty($reference['#feed_field_value'])) {
          unset($feed_field_values[$identifier][$index]);
        }
      }
    }
  }
  
  $feed_item_elements = feed_parser_retrieve_feed_item_elements($type, $elements);
  
  if ($feed_item_elements instanceof XmlElements && count($feed_item_elements)) {
    // Load feed_item_field into the feed_item_fields object array.
    if (isset($feed->fid)) {
      foreach (feed_load_fields($feed, TRUE) as $feed_item_field) {
        $identifier = sprintf('%s_%s', $feed_item_field->namespace, $feed_item_field->tag);
        $feed_item_fields[$identifier] = $feed_item_field;
      }
    }
    
    foreach ($feed_item_elements->reverse() as $feed_item_index => $feed_item_element) {
      $feed_item_indexes = array();
      
      // Try to load feed item object from element.
      $feed_items[$feed_item_index] = feed_parser_load_feed_item($type, $feed_item_element, $feed, isset($objects['feed_items'][$feed_item_index]) ? $objects['feed_items'][$feed_item_index] : NULL);

      if (isset($configs['feed_item_fields'][$feed_item_index])) {
        $feed_items[$feed_item_index]->config = $configs['feed_item_fields'][$feed_item_index];
      }
      // Initialize the feed_item_field_values object array.
      $feed_item_field_values[$feed_item_index] = array();
      
      if ($feed_item_element instanceof XmlElement) {
        $feed_item_field_elements = feed_parser_retrieve_feed_item_field_elements($type, $feed_item_element);
      
        if (!empty($feed_item_field_elements) && count($feed_item_field_elements)) {
          foreach ($feed_item_field_elements as $feed_item_field_element) {
            if ($feed_item_field_element instanceof XmlElement) {
              // Use namespace and tagname as identifier
              $identifier = sprintf('%s_%s', $feed_item_field_element->getTagNamespace(), $feed_item_field_element->getTag());
              
              // Keep track of indexes for cardinality of feed_item_field_values
              if (!isset($feed_item_indexes[$identifier])) {
                $feed_item_indexes[$identifier] = 0;
              }
              else {
                $feed_item_indexes[$identifier] += 1;
              }
              
              // Get the index of the feed item field value we will be creating/modifying
              $index = $feed_item_indexes[$identifier];
              
              // Load feed_item_field_values once into feed_item_field_values object array
              if (!isset($feed_item_field_values[$feed_item_index][$identifier])) {
                $feed_item_field_values[$feed_item_index][$identifier] = array();
              
                // Only load if feed item and feed item field is existing
                if (isset($feed_items[$feed_item_index]->fiid, $feed_item_fields[$identifier]->ffid)) {
                  foreach (feed_item_field_values_load($feed_items[$feed_item_index], $feed_item_fields[$identifier], TRUE) as $feed_field_value) {
                    $feed_item_field_values[$feed_item_index][$identifier][] = $feed_field_value;
                  }
                }
              }
              
              // Initialize the feed item field object if it does not exists.
              if (!isset($feed_item_fields[$identifier])) {
                if (isset($objects['feed_item_fields'][$identifier])) {
                  $feed_item_fields[$identifier] = $objects['feed_item_fields'][$identifier];
                }
                else {
                  $feed_item_fields[$identifier] = feed_field_defaults();
                }
              }
              
              // Initialize the feed item field value object if it does not exists.
              if (!isset($feed_item_field_values[$feed_item_index][$identifier][$index])) {
                if (isset($objects['feed_item_field_values'][$feed_item_index][$identifier][$index])) {
                  $feed_item_field_values[$feed_item_index][$identifier][$index] = $objects['feed_field_values'][$feed_item_index][$identifier][$index];
                }
                else {
                  $feed_item_field_values[$feed_item_index][$identifier][$index] = feed_item_field_value_defaults();
                }
                feed_object_set_data($feed_item_field_values[$feed_item_index][$identifier][$index], FEED_DATA_TYPE_FIELD_ATTRIBUTE, $feed_item_field_element->getNamespacedAttributes());
              }
              
              // Create the reference objects so we can modify these objects in invoker
              $reference = array();
              $reference['#feed'] = &$feed;
              $reference['#feed_item'] = &$feed_items[$feed_item_index];
              $reference['#feed_item_field'] = &$feed_item_fields[$identifier];
              $reference['#feed_item_field_value'] = &$feed_item_field_values[$feed_item_index][$identifier][$index];
              
              // Pull the configurations from the config array for this field
              $config = array();
              if (isset($feed_items[$feed_item_index]->config[$identifier])) {
                $config = $feed_items[$feed_item_index]->config[$identifier];
                
                if ($structure = feed_object_get_data($feed_item_fields[$identifier], FEED_DATA_FEED_FIELD_STRUCTURE)) {
                  $config['default'] = $structure;
                }
                elseif (!isset($config['default']) && !empty($config['types'])) {
                  $config['default'] = reset(array_keys($config['types']));
                }
              }
              feed_parser_parse_feed_item_field($type, $feed_item_field_element, $reference, $config);
              
              // Remove feed fields if feed_item_field reference does not exists anymore.
              if (empty($reference['#feed_item_field'])) {
                unset($feed_item_fields[$identifier]);
              }
              
              // Remove feed field values if the reference does not exists anymore.
              if (empty($reference['#feed_item_field_value'])) {
                unset($feed_item_field_values[$feed_item_index][$identifier]);
              }
            }
          }
        }
      }
    }
  }
  
  return array(
    'feed' => $feed,
    'feed_items' => $feed_items,
    'feed_fields' => $feed_fields,
    'feed_field_values' => $feed_field_values,
    'feed_item_fields' => $feed_item_fields,
    'feed_item_field_values' => $feed_item_field_values,
  );
}

/**
 * Calls the parser method to load feed
 * 
 * @param $type
 *   Parser type as specified by the key in hook_feed_parsers implementations.
 * @param $elements
 *   XmlElements object containing the feed.
 */
function feed_parser_load_feed($type, XmlElements $elements, $feed) {
  if ($parsed = feed_parser_invoke($type, 'load_feed', $elements, $feed)) {
    return feed_defaults($parsed);
  }
  return feed_defaults($feed);
}

/**
 * Calls the parser method to load feed item
 * 
 * @param $type
 *   Parser type as specified by the key in hook_feed_parsers implementations.
 * @param $elements
 *   XmlElements object containing the feed item.
 */
function feed_parser_load_feed_item($type, XmlElement $element, $feed, $feed_item = NULL) {
  if ($feed_item = feed_parser_invoke($type, 'load_feed_item', $element, $feed, $feed_item)) {
    return feed_item_defaults($feed_item);
  }
  return FALSE;
}

/**
 * Calls the parser method to retrieve feed field elements
 * 
 * @param $type
 *   Parser type as specified by the key in hook_feed_parsers implementations.
 * @param $elements
 *   XmlElements object containing the feed field elements.
 */
function feed_parser_retrieve_feed_field_elements($type, XmlElements $elements) {
  if ($return = feed_parser_invoke($type, 'retrieve_feed_field_elements', $elements)) {
    return $return;
  }
  watchdog('feed', 'Unable to retrieve feed field elements for type :type', array(':type' => $type));
}

/**
 * Calls the parser method to retrieve feed item elements
 * 
 * @param $type
 *   Parser type as specified by the key in hook_feed_parsers implementations.
 * @param $elements
 *   XmlElements object containing the feed item elements.
 */
function feed_parser_retrieve_feed_item_elements($type, XmlElements $elements) {
  if ($return = feed_parser_invoke($type, 'retrieve_feed_item_elements', $elements)) {
    return $return;
  }
  watchdog('feed', 'Unable to retrieve feed item elements for type :type', array(':type' => $type));
}

/**
 * Calls the parser method to retrieve feed item field elements
 * 
 * @param $type
 *   Parser type as specified by the key in hook_feed_parsers implementations.
 * @param $elements
 *   XmlElements object containing the feed item field elements.
 */
function feed_parser_retrieve_feed_item_field_elements($type, XmlElement $element) {
  if ($return = feed_parser_invoke($type, 'retrieve_feed_item_field_elements', $element)) {
    return $return;
  }
  watchdog('feed', 'Unable to retrieve feed item field elements for type :type', array(':type' => $type));
}

/**
 * Calls the parser method to parse feed info element
 * 
 * @param $type
 *   Parser type as specified by the key in hook_feed_parsers implementations.
 * @param $element
 *   XmlElement object containing the feed field element.
 * @param $objects
 *   An array of #feed, #feed_field, and #feed_field_value reference objects
 * @param $config
 *   Configurations for this feed field.
 */
function feed_parser_parse_feed_field($type, XmlElement $element, &$objects, $config) {
  $feed =& $objects['#feed'];
  $feed_field =& $objects['#feed_field'];
  $feed_field_value =& $objects['#feed_field_value'];
  
  // Set the default feed field name as the element tag.
  if (empty($feed_field->name)) {
    $name = $element->getTag();
    
    if ($element->getTagNamespace() != XmlElement::DEFAULT_NAMESPACE) {
      $name = $element->getTagNamespace() . " {$name}";
    }
    $feed_field->name = $name;
  }
  
  // Set the default feed field tag and namespace.
  if (empty($feed_field->tag)) {
    $feed_field->tag = $element->getTag();
  }
  if (empty($feed_field->namespace)) {
    $feed_field->namespace = $element->getTagNamespace();
  }
  
  // Create reference object and invoke 'parse_feed_info' method.
  $object = array('#feed' => &$feed, '#feed_field' => &$feed_field);
  if (!feed_parser_invoke($type, 'parse_feed_field', $element, $object, $config)) {
    unset($objects['#feed_field']);
    unset($objects['#feed_field_value']);
  }
  else {
    // Create reference object and invoke 'parse_feed_info_value' method.
    $object = array('#feed' => &$feed, '#feed_field' => &$feed_field, '#feed_field_value' => &$feed_field_value);
    if (!feed_parser_invoke($type, 'parse_feed_field_value', $element, $object, $config)) {
      unset($objects['#feed_field_value']);
    }
  }
}

/**
 * Calls the parser method to parse feed item field element
 * 
 * @param $type
 *   Parser type as specified by the key in hook_feed_parsers implementations.
 * @param $element
 *   XmlElement object containing the feed item field element.
 * @param $objects
 *   An array of #feed_item, #feed_item_field, and #feed_item_field_value reference objects
 * @param $config
 *   Configurations for this feed item field.
 */
function feed_parser_parse_feed_item_field($type, XmlElement $element, &$objects, $config) {
  $feed_item =& $objects['#feed_item'];
  $feed_item_field =& $objects['#feed_item_field'];
  $feed_item_field_value =& $objects['#feed_item_field_value'];

  // Set the default feed field name as the element tag.
  if (empty($feed_item_field->name)) {
    $name = $element->getTag();
    
    if ($element->getTagNamespace() != XmlElement::DEFAULT_NAMESPACE) {
      $name = $element->getTagNamespace() . " {$name}";
    }
    $feed_item_field->name = $name;
  }

  // Set the default feed field tag and namespace.
  if (empty($feed_item_field->tag)) {
    $feed_item_field->tag = $element->getTag();
  }
  if (empty($feed_item_field->namespace)) {
    $feed_item_field->namespace = $element->getTagNamespace();
  }
  
  // Create reference object and invoke 'parse_feed_info' method.
  $object = array('#feed_item' => &$feed_item, '#feed_item_field' => &$feed_item_field);
  if (!feed_parser_invoke($type, 'parse_feed_item_field', $element, $object, $config)) {
    unset($objects['#feed_item_field']);
    unset($objects['#feed_item_field_value']);
  }
  else {
    // Create reference object and invoke 'parse_feed_info_value' method.
    $object = array('#feed_item' => &$feed_item, '#feed_item_field' => &$feed_item_field, '#feed_item_field_value' => &$feed_item_field_value);
    if (!feed_parser_invoke($type, 'parse_feed_item_field_value', $element, $object, $config)) {
      unset($objects['#feed_item_field_value']);
    }
  }
}
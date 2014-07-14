<?php

/**
 * Implements hook_feed_PARSER_parser_types().
 */
function feed_feed_rss_parser_types() {
  $items['text_title'] = array(
    'type' => 'text',
    'configurable' => FALSE,
    'structures' => array(
      'text_title' => array(XmlElement::KEY_TAG => 'title'),
    ),
    'objects' => array(
      'feed_field_value' => array(
        'text' => array('feed_parser_structure_value' => array('text_title')),
      ),
      'feed_item_field_value' => array(
        'text' => array('feed_parser_structure_value' => array('text_title')),
      ),
    ),
  );
  $items['link_link'] = array(
    'type' => 'link',
    'configurable' => FALSE,
    'structures' => array(
      'link_link' => array(XmlElement::KEY_TAG => 'link'),
    ),
    'objects' => array(
      'feed_field_value' => array(
        'url' => array('feed_parser_structure_value' => array('link_link')),
      ),
      'feed_item_field_value' => array(
        'url' => array('feed_parser_structure_value' => array('link_link')),
      ),
    ),
  );
  $items['html_description'] = array(
    'type' => 'html',
    'configurable' => FALSE,
    'structures' => array(
      'html_description' => array(XmlElement::KEY_TAG => 'description'),
    ),
    'objects' => array(
      'feed_field_value' => array(
        'html' => array('feed_parser_structure_value' => array('html_description')),
      ),
      'feed_item_field_value' => array(
        'html' => array('feed_parser_structure_value' => array('html_description')),
      ),
    ),
  );
  $items['date_publish'] = array(
    'type' => 'date',
    'configurable' => FALSE,
    'structures' => array(
      'date_pubdate' => array(XmlElement::KEY_TAG => 'pubDate'),
    ),
    'objects' => array(
      'feed_field_value' => array(
        'timestamp' => array('feed_parser_rss_date_publish_timestamp_value' => array('date_pubdate')),
        'timezone' => array('feed_parser_rss_date_publish_timezone_value' => array('date_pubdate')),
      ),
      'feed_item_field_value' => array(
        'timestamp' => array('feed_parser_rss_date_publish_timestamp_value' => array('date_pubdate')),
        'timezone' => array('feed_parser_rss_date_publish_timezone_value' => array('date_pubdate')),
      ),
    ),
  );
  $items['text'] = array(
    'type' => 'text',
    'configurable' => TRUE,
    'structures' => array('text' => array()),
    'objects' => array(
      'feed_field_value' => array(
        'text' => array('feed_parser_structure_value' => array('text')),
      ),
      'feed_item_field_value' => array(
        'text' => array('feed_parser_structure_value' => array('text')),
      ),
    ),
  );
  return $items;
}

/**
 * Configuration options for feed item field.
 */
function feed_parser_rss_config_field(XmlElement $element) {
  $config = array();
  $config['default'] = NULL;
  $config['types'] = array();
  $config['elements'] = array();
  $config['structures'] = array();
  
  $element_structure = $element->toArray();
  $parser_type_infos = feed_parser_type_info('rss');
  
  foreach ($parser_type_infos as $name => $type) {
    $match = array();
  
    if (!empty($type['structures']) && is_array($type['structures'])) {
      feed_parser_validate_structure($element_structure, $type['structures'], $match);
    }
    
    if (!empty($match)) {
      $config['elements'][$name] = $match;
      $config['structures'][$name] = $type;
  
      if (isset($type['type']) && !in_array($type['type'], $config['types'])) {
        $config['types'][$name] = $type['type'];
      }
    }
  }
  return $config;
}

/**
 * Loads a feed object from a feed element
 */
function feed_parser_rss_load_feed(XmlElements $elements, $feed = NULL) {
  $feed = feed_defaults($feed);
  
  $namespaces = $elements->getFirst()->getNamespaces();
  
  if (!empty($namespaces) && is_array($namespaces)) {
    feed_object_set_data($feed, FEED_DATA_FEED_NAMESPACE, $namespaces);
  }
  return $feed;
}

/**
 * Loads a feed item object from a feed item element
 */
function feed_parser_rss_load_feed_item(XmlElement $element, $feed, $feed_item = NULL) {
  $feed_item = feed_item_defaults($feed_item);
  
  if (!empty($feed->fid)) {
	  foreach (array('guid', 'link', 'title') as $tagname) {
	    if (!$elem = $element->getChildByTag($tagname)) {
	      continue;
	    }
	    if ($feed_item_field = feed_item_field_tagname_load($tagname, $feed->fid)) {
	      switch ($feed_item_field->type) {
	        case 'text':
	          $feed_items = feed_item_load_from_field_value($feed_item_field, 'text', $elem->getValue());
	        break;
	        
	        case 'link':
	          $feed_items = feed_item_load_from_field_value($feed_item_field, 'url', $elem->getValue());
	        break;
	        
	        case 'html':
	          $feed_items = feed_item_load_from_field_value($feed_item_field, 'html', $elem->getValue());
	        break;
	      }
	      
	      if (!empty($feed_items)) {
	        $loaded_feed_item = reset($feed_items);
	      }
	    }
	  }
	  if (!empty($loaded_feed_item)) {
	    foreach ($loaded_feed_item as $key => $value) {
	      $feed_item->{$key} = $value;
	    }
	  }
  }
  return $feed_item;
}

/**
 * Retrieve RSS feed info elements.
 */
function feed_parser_rss_retrieve_feed_field_elements(XmlElements $elements) {
  $infos = new XmlElements();
  
  foreach ($elements as $element) {
    foreach ($element->findByTag('channel') as $channel) {
      $infos->addElements($channel->getChildrenNotTag('item'));
    }
  }
  return $infos;
}

/**
 * Retrieve RSS feed item elements.
 */
function feed_parser_rss_retrieve_feed_item_elements(XmlElements $elements) {
  $items = new XmlElements();
  
  foreach ($elements as $element) {
    foreach ($element->findByTag('channel') as $channel) {
      $items->addElements($channel->getChildrenByTag('item'));
    }
  }
  return $items;
}

/**
 * Retrieve RSS feed item field elements.
 */
function feed_parser_rss_retrieve_feed_item_field_elements(XmlElement $element) {
  return $element->getChildren();
}

/**
 * Parse RSS feed field element.
 */
function feed_parser_rss_parse_feed_field(XmlElement $element, $objects, $config) {
  $feed =& $objects['#feed'];
  $feed_field =& $objects['#feed_field'];
  
  $type = $config['default'];
  
  if (!$info = feed_parser_type_info('rss', $type, TRUE)) {
    watchdog('feed', 'Cannot parse feed field %field, parser field type %type could not be found', array('%type' => $config['default']));
    return FALSE;
  }
  $feed_field->type = $info['type'];
  
  $objects = array('feed' => $feed, 'feed_field' => $feed_field);
  foreach ($objects as $object_name => &$object) {
    if (isset($config['elements'][$type])) {
      $values = feed_parser_type_process_object('rss', $type, $object_name, $config['elements'][$type]);
    }
    
    if (!empty($values)) {
      foreach ($values as $name => $value) {
        $object->{$name} = $value;
      }
    }
  }
  return !isset($info['feed_field_value']) || $info['feed_field_value'] !== FALSE;
}


/**
 * Parse RSS feed field values.
 */
function feed_parser_rss_parse_feed_field_value(XmlElement $element, $objects, $config) {
  $feed =& $objects['#feed'];
  $feed_field =& $objects['#feed_field'];
  $feed_field_value =& $objects['#feed_field_value'];
  $type = $config['default'];
  
  if (isset($config['elements'][$type])) {
    $values = feed_parser_type_process_object('rss', $type, 'feed_field_value', $config['elements'][$type]);
  }
  
  if (!empty($values)) {
    feed_field_value_apply($feed_field, $feed_field_value, $values);
    return TRUE;
  }
  return FALSE;
}

/**
 * Parse RSS feed item field element.
 */
function feed_parser_rss_parse_feed_item_field(XmlElement $element, $objects, $config) {
  $feed_item =& $objects['#feed_item'];
  $feed_item_field =& $objects['#feed_item_field'];
  $type = $config['default'];
  
  if (!$info = feed_parser_type_info('rss', $type, TRUE)) {
    watchdog('feed', 'Cannot parse feed field %field, parser field type %type could not be found', array('%type' => $config['default']));
    return FALSE;
  }
  $feed_item_field->type = $info['type'];
  
  $objects = array('feed_item' => $feed_item, 'feed_item_field' => $feed_item_field);
  foreach ($objects as $object_name => &$object) {
    if (isset($config['elements'][$type])) {
      $values = feed_parser_type_process_object('rss', $type, $object_name, $config['elements'][$type]);
    }
    
    if (!empty($values)) {
      foreach ($values as $name => $value) {
        $object->{$name} = $value;
      }
    }
  }
  return !isset($info['feed_item_field_value']) || $info['feed_item_field_value'] !== FALSE;
}

/**
 * Parse RSS feed item field value.
 */
function feed_parser_rss_parse_feed_item_field_value(XmlElement $element, $objects, $config) {
  $feed_item =& $objects['#feed_item'];
  $feed_item_field =& $objects['#feed_item_field'];
  $feed_item_field_value =& $objects['#feed_item_field_value'];
  $type = $config['default'];
  
  if (isset($config['elements'][$type])) {
    $values = feed_parser_type_process_object('rss', $type, 'feed_item_field_value', $config['elements'][$type]);
  }
  if (!empty($values)) {
    feed_item_field_value_apply($feed_item_field, $feed_item_field_value, $values);
    return TRUE;
  }
  return FALSE;
}

/**
 * Parses a date timestamp value from a RSS publish date value
 */
function feed_parser_rss_date_publish_timestamp_value($options, $arguments) {
  if ($datestring = feed_parser_structure_value($options, $arguments)) {
    return ($date = DateTime::createFromFormat(DATE_RSS, $datestring)) ? $date->getTimestamp() : NULL;
  }
  return NULL;
}

/**
 * Parses a date timestamp value from a RSS publish date value
 */
function feed_parser_rss_date_publish_timezone_value($options, $arguments) {
  if ($datestring = feed_parser_structure_value($options, $arguments)) {
    if ($date = DateTime::createFromFormat(DATE_RSS, $datestring)) {
      $offset = $date->getOffset() / (60 * 60);
      
      if (is_integer($offset)) {
        if ($offset > 0) {
          return 'gmt+' . abs($offset);
        }
        elseif ($offset < 0) {
          return 'gmt-' . abs($offset);
        }
      }
    }
  }
  return NULL;
}
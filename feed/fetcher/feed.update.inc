<?php 

/**
 * Retrieve feeds for the cron.
 */
function feed_update_queue() {
  $feeds = feed_load_all(FEED_TYPE_IMPORTED, NULL, TRUE);
  
  $queues = array();
  
  foreach ($feeds as $feed) {
    $refresh = feed_object_get_data($feed, FEED_DATA_FEED_UPDATE_REFRESH);
    $queued = feed_object_get_data($feed, FEED_DATA_FEED_UPDATE_QUEUED);
    
    if (!$queued && REQUEST_TIME <= $refresh) {
      $queues[] = $feed;
    }
  }
  return $queues;
}

/**
 * Updates a feed for new feed items
 * 
 * @param $feed
 *   A feed object.
 */
function feed_updater($feed) {
  $url = feed_object_get_data($feed, FEED_DATA_FEED_UPDATE_URL);
  $frequency = feed_object_get_data($feed, FEED_DATA_FEED_UPDATE_FREQ);
  $refresh = feed_object_get_data($feed, FEED_DATA_FEED_UPDATE_REFRESH);
  $parsertype = feed_object_get_data($feed, FEED_DATA_FEED_PARSER);
  
  if ($response = feed_fetch($feed, $url)) {
    $parser = create_xml_parser($response->data);
    
    if ($parser && $elements = $parser->parse()) {
      $configs = feed_config_parse($parsertype, $elements);
      $objects = feed_parse(array('feed' => $feed), $parsertype, $elements, $configs);
      
      feed_object_set_data($objects['feed'], FEED_DATA_FEED_UPDATE_QUEUED, FALSE);
      feed_object_set_data($objects['feed'], FEED_DATA_FEED_UPDATE_REFRESH, (int) $refresh + (int) $frequency);
      
      feed_build_save($objects);
    }
    else {
      watchdog('feed', 'Unable to parse the response from URL :url for feed :fid', array(':url' => $url, ':fid' => $feed->fid));
    }
  }
  else {
    watchdog('feed', 'Unable to get response from URL :url for feed :fid', array(':url' => $url, ':fid' => $feed->fid));
  }
}
<?php

/**
 * Retrieve subscribe feeds for the cron.
 */
function feed_subscribe_queue() {
  $feeds = feed_load_all(FEED_TYPE_SUBSCRIBED, NULL, TRUE);
  
  $queues = array();
  
  foreach ($feeds as $feed) {
    $interval = feed_object_get_data($feed, FEED_DATA_FEED_SUBSCRIBE_INTERVAL);
    $refresh = feed_object_get_data($feed, FEED_DATA_FEED_SUBSCRIBE_REFRESH);
    $queued = feed_object_get_data($feed, FEED_DATA_FEED_SUBSCRIBE_QUEUED);

    if (!$queued && REQUEST_TIME >= ($refresh + $interval)) {
      $queues[] = $feed;
    }
  }
  return $queues;
}

/**
 * Subscribes new feed items for a feed subscription
 */
function feed_subscriber($feed, array $filters = NULL) {
  $interval = feed_object_get_data($feed, FEED_DATA_FEED_SUBSCRIBE_INTERVAL, 3600 * 3);
  $refresh = feed_object_get_data($feed, FEED_DATA_FEED_SUBSCRIBE_REFRESH_FILTER, array());
  
  if (empty($filters)) {
    $filters = feed_subscription_filter_load_all_feed($feed->fid);
  }

  if (!empty($filters)) {
    foreach ($filters as $filter) {
      if (!empty($refresh[$filter->fsfid]) && (REQUEST_TIME + $interval) < $refresh[$filter->fsfid]) {
        continue;
      }
      $config = $filter->configuration;
      
      $pager = new FeedItemPager();
      
      if (!empty($refresh[$filter->fsfid])) {
        $pager->getFeedItemQuery()->condition('fi.updated_at', $refresh[$filter->fsfid], '>');
      }
      
      if (in_array($config['feed']['option'], array('criteria-select', 'subscribe-select'))) {
        $pager->hasFeedCreated($config['feed']['select']);
      }
      
      
      if (in_array($config['feed']['option'], array('criteria-all', 'criteria-select'))) {
        if (!empty($config['feed']['group'])) {
          $pager->hasFeedGroupId($config['feed']['group']);
        }
        
        foreach ($config['feed_item']['terms'] as $fcid => $fctids) {
          if (!empty($fctids)) {
            $pager->hasFeedItemTagIds($fctids);
          }
        }
        
        if (!empty($config['feed_item']['fields'])) {
          foreach ($config['feed_item']['fields'] as $tagname => $term) {
            if (!$feed_item_fields = feed_item_field_tagname_load($tagname)) {
              continue;
            }
            $field_condition = db_or();
            foreach ($feed_item_fields as $feed_item_field) {
              switch ($feed_item_field->type) {
                case 'text':
                case 'html':
                  if (preg_match('/^[A-Za-z0-9-_",\'\s]+$/', $term)) {
                    $field_condition->condition(db_and()
                      ->condition('fifvd.name', 'text')
                      ->condition('fifvd.data', '%' . db_like($term) . '%', 'LIKE')
                    );
                  }
                break;
                
                case 'date':
                  $field_condition->condition(db_and()
                    ->condition('fifvd.name', 'timestamp')
                    ->condition('fifvd.data', intval($term), '>=')
                  );
                break;
                
                case 'link':
                  if (preg_match('/^[A-Za-z0-9-_",\'\s]+$/', $term)) {
                    $field_condition->condition(db_and()
                      ->condition('fifvd.name', 'url')
                      ->condition('fifvd.data', '%' . db_like($term) . '%', 'LIKE')
                    );
                  }
                break;
              }
            }
            
            if ($field_condition->count() > 0) {
              $pager->hasFieldConditionByTagname($tagname, $field_condition);
            }
          }
        }
      }
      $pager->hasFieldValueByTagname('pubDate', array('timestamp' => '0'), '>');
      
      $feed_items = $pager->query();
  
      if (!empty($feed_items)) {
        feed_subscribe_feed_items($feed, $feed_items);
      }
      $refresh[$filter->fsfid] = REQUEST_TIME;
    }

    feed_object_set_data($feed, FEED_DATA_FEED_SUBSCRIBE_REFRESH, REQUEST_TIME);
    feed_object_set_data($feed, FEED_DATA_FEED_SUBSCRIBE_REFRESH_FILTER, $refresh);
    feed_object_set_data($feed, FEED_DATA_FEED_SUBSCRIBE_QUEUED, FALSE);
    feed_save($feed);
  }
}

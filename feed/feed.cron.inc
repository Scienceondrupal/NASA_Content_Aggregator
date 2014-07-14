<?php

/**
 * Implements hook_cron().
 */
function feed_cron() {
  $update_queue = DrupalQueue::get('feed_updater');
  foreach (feed_update_queue() as $feed_update) {
    if ($update_queue->createItem($feed)) {
      feed_object_set_data($feed_update, FEED_DATA_FEED_UPDATE_QUEUED, TRUE);
      feed_save($feed_update);
    }
  }
  
  $subscribe_queue = DrupalQueue::get('feed_subscriber');
  foreach (feed_subscribe_queue() as $feed_subscribe) {
    if ($subscribe_queue->createItem($feed_subscribe)) {
      feed_object_set_data($feed_subscribe, FEED_DATA_FEED_SUBSCRIBE_QUEUED, TRUE);
      feed_save($feed_subscribe);
    }
  }
  
  $email_queue = DrupalQueue::get('feed_emailer');
  foreach (feed_email_queue() as $feed_email) {
    if ($email_queue->createItem($feed_email)) {
      $feed_email->configuration[FEED_EMAIL_CONFIG_QUEUED] = TRUE;
      feed_email_save($feed_email);
    }
  }
}

/**
 * Implements hook_cron_queue_info().
 */
function feed_cron_queue_info() {
  $items['feed_updater'] = array(
    'worker callback' => 'feed_updater',
    'time' => 3600,
  );
  $items['feed_subscriber'] = array(
    'worker callback' => 'feed_subscriber',
    'time' => 3600 * 3,
  );
  $items['feed_emailer'] = array(
    'worker callback' => 'feed_emailer',
    'time' => 3600 * 3,
  );
  return $items;
}
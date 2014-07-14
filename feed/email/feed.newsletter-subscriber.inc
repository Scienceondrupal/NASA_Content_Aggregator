<?php

/**
 * Load a feed newsletter subscriber.
 *
 * @param $fnsid
 *   The feed newsletter subscriber id.
 * @param $reload (optional)
 *   A boolean to indicate whether to reload data from database.
 *   
 * @return
 *   An object describing the feed newsletter subscriber.
 */
function feed_newsletter_subscriber_load($fnsid, $reload = FALSE) {
  $feed_newsletter_subscribers = &drupal_static(__FUNCTION__);
  
  if (!isset($feed_newsletter_subscribers)) {
    $feed_newsletter_subscribers = array();
  }
  if ($reload || !isset($feed_newsletter_subscribers[$fnsid])) {
    $feed_newsletter_subscribers[$fnsid] = db_query('SELECT * FROM {feed_newsletter_subscriber} WHERE fnsid = :fnsid', array(':fnsid' => $fnsid))->fetchObject();

    if (is_string($feed_newsletter_subscribers[$fnsid]->configuration) && $config = @unserialize($feed_newsletter_subscribers[$fnsid]->configuration)) {
      $feed_newsletter_subscribers[$fnsid]->configuration = $config;
    }
  }
  return $feed_newsletter_subscribers[$fnsid];
}

/**
 * Return an array of all feed newsletter subscribers belonging to feed newsletter
 * 
 * @param $feed_newsletter
 *   The feed newsletter to load feed newsletter subscriber objects for.
 * @param $reload (optional)
 *   A boolean to indicate whether to reload data from database.
 *   
 * @return
 *   An array of all feed newsletter subscirber objects, indexed by fnsid.
 */
function feed_newsletter_subscribers_load_all($feed_newsletter, $reload = FALSE) {
  $feed_newsletter_subscribers = drupal_static(__FUNCTION__, array());

  if ($reload || !isset($feed_newsletter_subscribers[$feed_newsletter->fnid])) {
    $feed_newsletter_subscribers[$feed_newsletter->fnid] = db_select('feed_newsletter_subscriber', 'fns')
      ->fields('fns')
      ->condition('fns.fnid', $feed_newsletter->fnid)
      ->execute()->fetchAllAssoc('fnsid');
      
    foreach ($feed_newsletter_subscribers[$feed_newsletter->fnid] as &$feed_newsletter_subscriber) {
      if (is_string($feed_newsletter_subscriber->configuration) && $config = @unserialize($feed_newsletter_subscriber->configuration)) {
        $feed_newsletter_subscriber->configuration = $config;
      }
    }
  }
  return $feed_newsletter_subscribers[$feed_newsletter->fnid];
}

/**
 * Save a feed newsletter subscriber.
 *
 * @param $feed_newsletter_subscriber
 *   The feed newsletter subscriber to save.
 * @param $feed_newsletter
 *   The feed newsletter object to subscribe to.
 * @param $transaction (optional)
 *   A DatabaseTransaction object.
 *
 * @return
 *   Status flag indicating outcome of the operation.
 */
function feed_newsletter_subscriber_save($feed_newsletter_subscriber, $feed_newsletter, DatabaseTransaction $transaction = NULL) {
  if (!$transaction) {
    $transaction = db_transaction();
  }
  
  try {
    if (!isset($feed_newsletter->fnid)) {
      throw new Exception('Could not save feed newsletter subscriber record, invalid feed newsletter');
    }
    $feed_newsletter_subscriber->fnid = $feed_newsletter->fnid;
    
    if (!$status = drupal_write_record('feed_newsletter_subscriber', $feed_newsletter_subscriber, isset($feed_newsletter_subscriber->fnsid) ? 'fnsid' : array())) {
      throw new Exception('Could not save feed newsletter subscriber record');
    }
    db_ignore_slave();
    
    return $status;
  }
  catch (Exception $e) {
    $transaction->rollback();
    watchdog_exception('feed', $e);
    return FALSE;
  }
}

/**
 * Deletes a feed newsletter subscriber from the database.
 *
 * @param $feed_newsletter_subscriber
 *   The feed newsletter subscriber to delete, as an object.
 * @param $transaction (optional)
 *   A DatabaseTransaction object.
 */
function feed_newsletter_subscriber_delete($feed_newsletter_subscriber, DatabaseTransaction $transaction = NULL) {
  if (!$transaction) {
    $transaction = db_transaction();
  }

  try {
    if (isset($feed_newsletter_subscriber->fnsid)) {
      db_delete('feed_newsletter_subscriber')
        ->condition('fnsid', $feed_newsletter_subscriber->fnsid)
        ->execute();
    }
    return TRUE;
  }
  catch (Exception $e) {
    $transaction->rollback();
    watchdog_exception('feed', $e);
    return FALSE;
  }
}

/**
 * Configure feed newsletter subscriber object/ensure defaults
 * 
 * @param $feed_newsletter_subscriber
 *   A feed newsletter subscriber object.
 */
function feed_newsletter_subscriber_defaults($feed_newsletter_subscriber = NULL) {
  $feed_newsletter_subscriber = (object) $feed_newsletter_subscriber;
  
  if (!isset($feed_newsletter_subscriber->email)) $feed_newsletter_subscriber->email = '';
  if (!isset($feed_newsletter_subscriber->configuration)) $feed_newsletter_subscriber->configuration = array();
  
  return $feed_newsletter_subscriber;
}
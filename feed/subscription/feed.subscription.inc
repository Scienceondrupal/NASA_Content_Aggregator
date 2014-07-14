<?php 

/**
 * Subscribes a feed to a feed item.
 * 
 * @param $feed
 *   A feed object.
 * @param $feed_item
 *   A feed item object.
 */
function feed_subscribe_feed_item($feed, $feed_item, DatabaseTransaction $transaction = NULL) {
  if (!$transaction) {
    $transaction = db_transaction();
  }

  try {
    if (isset($feed_item->fiid) && isset($feed->fid)) {
      return db_merge('feed_item_subscription')
        ->key(array('fid' => $feed->fid, 'fiid' => $feed_item->fiid))
        ->fields(array('fid' => $feed->fid, 'fiid' => $feed_item->fiid))
        ->execute();
    }
    return FALSE;
  }
  catch (Exception $e) {
    $transaction->rollback();
    watchdog_exception('feed', $e);
    return FALSE;
  }
}

/**
 * Subscribes a feed to feed items.
 * 
 * @param $feed
 *   A feed object.
 * @param $feed_items
 *   An array of feed item object.
 */
function feed_subscribe_feed_items($feed, array $feed_items, DatabaseTransaction $transaction = NULL) {
  if (!$transaction) {
    $transaction = db_transaction();
  }

  try {
    if (isset($feed->fid)) {
      foreach ($feed_items as $feed_item) {
        if (isset($feed_item->fiid)) {
          $status = db_merge('feed_item_subscription')
            ->key(array('fid' => $feed->fid, 'fiid' => $feed_item->fiid))
            ->fields(array('fid' => $feed->fid, 'fiid' => $feed_item->fiid))
            ->execute();
            
          if (!$status) {
            throw new Exception('Could not complete merge operation.');
          }
        }
      }
      return TRUE;
    }
    return FALSE;
  }
  catch (Exception $e) {
    $transaction->rollback();
    watchdog_exception('feed', $e);
    return FALSE;
  }
}

/**
 * Unsubscribes a feed to a feed item.
 * 
 * @param $feed
 *   A feed object.
 * @param $feed_item
 *   A feed item object.
 */
function feed_unsubscribe_feed_item($feed, $feed_item, DatabaseTransaction $transaction = NULL) {
  if (!$transaction) {
    $transaction = db_transaction();
  }

  try {
    if (isset($feed_item->fiid) && isset($feed->fid)) {
      db_delete('feed_item_subscription')
        ->condition('fid', $feed->fid)
        ->condition('fiid', $feed_item->fiid)
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
 * Unsubscribes a feed to a feed item.
 * 
 * @param $feed
 *   A feed object.
 * @param $feed_item
 *   A feed item object.
 */
function feed_unsubscribe_feed_items($feed, $feed_items, DatabaseTransaction $transaction = NULL) {
  if (!$transaction) {
    $transaction = db_transaction();
  }

  try {
    if (isset($feed->fid)) {
      $fiids = array();
      foreach ($feed_items as $feed_item) {
        if (isset($feed_item->fiid)) {
          $fiids[$feed_item->fiid] = $feed_item->fiid;
        }
      }
      
      db_delete('feed_item_subscription')
        ->condition('fid', $feed->fid)
        ->condition('fiid', $fiids)
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
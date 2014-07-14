<?php

/**
 * Feed access callback;
 */
function feed_admin_access() {
  $arguments = func_get_args();
  $feed = array_shift($arguments);
  foreach ($arguments as $access) {
    if (!user_access($access)) {
      return FALSE;
    }
  }
  return feed_access($feed);
}

/**
 * Feed access checker;
 */
function feed_access($feed) {
  global $user;
  
  if (isset($feed->uid)) {
    return $feed->uid == $user->uid;
  }
  return FALSE;
}

/**
 * Feed category access callback;
 */
function feed_category_admin_access() {
  $arguments = func_get_args();
  $allow_public = array_shift($arguments);
  $feed_category = array_shift($arguments);
  foreach ($arguments as $access) {
    if (!user_access($access)) {
      return FALSE;
    }
  }
  return feed_category_access($allow_public, $feed_category);
}

/**
 * Feed category access checker;
 */
function feed_category_access($allow_public = FALSE, $feed_category) {
  global $user;
  
  if ($allow_public && $feed_category->is_public) {
    return TRUE;
  }
  if (isset($feed_category->uid)) {
    return $feed_category->uid == $user->uid;
  }
  return FALSE;
}

/**
 * Feed category term access callback;
 */
function feed_category_term_admin_access() {
  $arguments = func_get_args();
  $feed_category_term = array_shift($arguments);
  foreach ($arguments as $access) {
    if (!user_access($access)) {
      return FALSE;
    }
  }
  return feed_category_term_access($feed_category_term);
}

/**
 * Feed category public access checker;
 */
function feed_category_term_access($feed_category_term) {
  global $user;
  
  if (isset($feed_category_term->uid) && $feed_category_term->uid == $user->uid) {
    return TRUE;
  }
  if (isset($feed_category_term->fcid) && $feed_category = feed_category_load($feed_category_term->fcid)) {
    return $feed_category->uid == $user->uid;
  }
  return FALSE;
}

/**
 * Feed widget access callback;
 */
function feed_widget_admin_access() {
  $arguments = func_get_args();
  $feed = array_shift($arguments);
  foreach ($arguments as $access) {
    if (!user_access($access)) {
      return FALSE;
    }
  }
  return feed_widget_access($feed);
}

/**
 * Feed access checker;
 */
function feed_widget_access($feed_widget) {
  global $user;
  
  if (isset($feed_widget->uid)) {
    return $feed_widget->uid == $user->uid;
  }
  return FALSE;
}

/**
 * Feed newsletter access callback;
 */
function feed_newsletter_admin_access() {
  $arguments = func_get_args();
  $feed_newsletter = array_shift($arguments);
  foreach ($arguments as $access) {
    if (!user_access($access)) {
      return FALSE;
    }
  }
  return feed_newsletter_access($feed_newsletter);
}

/**
 * Feed access checker;
 */
function feed_newsletter_access($feed_newsletter) {
  global $user;
  
  if (isset($feed_newsletter->uid)) {
    return $feed_newsletter->uid == $user->uid;
  }
  return FALSE;
}

/**
 * Retrieve all feed fids this user has access to.
 * 
 * @param $user
 *   A User object. Current user if none given.
 */
function feed_access_feeds($user = NULL) {
  if (!$user) {
    global $user;
  }
  $fids = &drupal_static(__FUNCTION__);
  
  if (!isset($user->uid)) {
    return array();
  }
  if (!isset($fids[$user->uid])) {
    $fids[$user->uid] = db_select('feed', 'f')
      ->fields('f', array('fid'))
      ->condition('f.uid', $user->uid)
      ->execute()->fetchAllKeyed(0, 0);
  }
  return $fids[$user->uid];
}
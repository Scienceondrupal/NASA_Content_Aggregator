<?php

/**
 * Fetches a feed xml from a url.
 *
 * @param $feed
 *   A feed object.
 */
function feed_fetch(&$feed, &$url, $etag = NULL, $modified = NULL) {
  $headers = array();

  if (!empty($etag)) {
    $headers['If-None-Match'] = $etag;
  }
  if (!empty($modified)) {
    $headers['If-Modified-Since'] = gmdate(DATE_RFC1123, $modified);
  }

  // Request feed.
  $result = drupal_http_request($url, array('headers' => $headers));

  // Process HTTP response code.
  switch ($result->code) {
    case 304:
    break;
    
    case 301:
      $url = $result->redirect_url;
    case 200:
    case 302:
    case 307:
      if (!isset($result->data)) {
        $result->data = '';
      }
      if (!isset($result->headers)) {
        $result->headers = array();
      }
      return $result;
    break;

    default:
      watchdog('feed', 'The feed seems to be broken, due to "%error".', array('%error' => $result->code . ' ' . $result->error), WATCHDOG_WARNING);
      drupal_set_message(t('The feed seems to be broken, because of error "%error".', array('%error' => $result->code . ' ' . $result->error)));
  }
  return FALSE;
}
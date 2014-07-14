<?php

$mailer = variable_get('mail_system', array('default-system' => 'DefaultMailSystem'));
if (empty($mailer['feed'])) {
  $mailer['feed'] = 'FeedMailSystem';
  variable_set('mail_system', $mailer);
}

/**
 * Retrieve feed emails for the cron.
 */
function feed_email_queue() {
  $queues = array();
  
  foreach (feed_newsletter_load_all() as $feed_newletter) {
    foreach (feed_newsletter_subscribers_load_all($feed_newletter) as $feed_newsletter_subscriber) {
      $interval = feed_object_get_configuration($feed_newsletter_subscriber, FEED_EMAIL_CONFIG_INTERVAL);
      $queued = feed_object_get_configuration($feed_newsletter_subscriber, FEED_EMAIL_CONFIG_QUEUED);
      $refresh = feed_object_get_configuration($feed_newsletter_subscriber, FEED_EMAIL_CONFIG_REFRESH);
      
      if (!$queued && REQUEST_TIME >= ($refresh + $interval)) {
        $queues[] = $feed_newsletter_subscriber;
      }
    }
  }
  return $queues;
}

/**
 * Implement hook_mail().
 */
function feed_mail($key, &$message, $param) {
  switch ($key) {
    case 'newsletter':
      $message['subject'] = t('Earthdata Newsletter: @name', array('@name' => $param['name']));
      
      if (!empty($param['html'])) {
        $message['body'][] = $param['html'];
        $message['headers']['Content-Type'] = 'text/html; charset=UTF-8; format=flowed';
      }
      if (!empty($param['text'])) {
        $message['body'][] = $param['text'];
      }
    break;
  }
}

/**
 * Emails new feed items for a feed newsletter
 */
function feed_emailer($subscriber) {
  if (!$newsletter = feed_newsletter_load($subscriber->fnid)) {
    return FALSE;
  }
  
  $interval = feed_object_get_configuration($subscriber, FEED_EMAIL_CONFIG_INTERVAL);
  $queued = feed_object_get_configuration($subscriber, FEED_EMAIL_CONFIG_QUEUED);
  $refresh = feed_object_get_configuration($subscriber, FEED_EMAIL_CONFIG_REFRESH);

  $mail = drupal_mail('feed', 'newsletter', $subscriber->email, language_default(), array(
    'name' => $newsletter->name,
    'html' => feed_generate_newsletter($subscriber, $refresh, $refresh + $interval),
  ));

  if (!empty($mail['result'])) {
    feed_object_set_configuration($subscriber, FEED_EMAIL_CONFIG_REFRESH, REQUEST_TIME);
    feed_object_set_configuration($subscriber, FEED_EMAIL_CONFIG_QUEUED, FALSE);
    
    return feed_newsletter_subscriber_save($subscriber, $newsletter);
  }
  return FALSE;
}

/**
 * Generates newsletter for feed
 */
function feed_generate_newsletter($subscriber, $start = NULL, $stop = NULL) {
  if (!$newsletter = feed_newsletter_load($subscriber->fnid)) {
    return FALSE;
  }
  $start = $start === NULL ? 0 : $start;
  $stop = $stop === NULL ? REQUEST_TIME : $stop;
  
  $config = $newsletter->configuration;
  $pager = new FeedItemPager();
  
  if (!empty($start)) {
    $pager->getFeedItemQuery()->condition('fi.updated_at', $start, '>=');
  }
  if (!empty($stop)) {
    $pager->getFeedItemQuery()->condition('fi.updated_at', $stop, '<=');
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
  
  $values = array();
  $values['name'] = $newsletter->name;
  
  $values['feeds'] = array();
  foreach ($pager->query() as $feed_item) {
    if (!isset($values['feeds'][$feed_item->fid])) {
      $values['feeds'][$feed_item->fid] = array();
      $values['feeds'][$feed_item->fid]['title'] = feed_field_values_tagname_format($feed_item->fid, 'title', 'text');
      $values['feeds'][$feed_item->fid]['link'] = feed_field_values_tagname_format($feed_item->fid, 'link', 'text');
      $values['feeds'][$feed_item->fid]['description'] = feed_field_values_tagname_format($feed_item->fid, 'description', 'text');
      $values['feeds'][$feed_item->fid]['items'] = array();
    }
    
    $values['feeds'][$feed_item->fid]['items'][$feed_item->fiid] = array(
      'title' => feed_item_field_values_tagname_format($feed_item->fiid, 'title', 'text'),
      'link' => feed_item_field_values_tagname_format($feed_item->fiid, 'link', 'text'),
      'description' => feed_item_field_values_tagname_format($feed_item->fiid, 'description', 'text'),
      'date' => feed_item_field_values_tagname_format($feed_item->fiid, 'pubDate', 'text'),
    );
  }
  return theme('feed_newsletter', $values);
}

class FeedMailSystem extends DefaultMailSystem {
  
  /**
   * Concatenate and wrap the e-mail body for plain-text mails.
   *
   * @param $message
   *   A message array, as described in hook_mail_alter().
   *
   * @return
   *   The formatted $message.
   */
  public function format(array $message) {
    $message['body'] = implode("\n\n", $message['body']);
    
    if (!isset($message['headers']['Content-Type']) || !preg_match('/text\/html/i', $message['headers']['Content-Type'])) {
      $message['body'] = drupal_html_to_text($message['body']);
    }
    $message['body'] = drupal_wrap_mail($message['body']);
    return $message;
  }
}

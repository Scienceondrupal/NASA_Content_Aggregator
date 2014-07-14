<?php

/**
 * Form Handler; Feed subscription filter form;
 */
function feed_subscription_filter_form($form, &$form_state, $feed_subscription, $feed_subscription_filter = NULL) {
  $form_state['#feed'] = $feed_subscription;
  $form_state['#feed_subscription_filter'] = feed_subscription_filter_defaults($feed_subscription_filter);
  
  if (empty($form_state['#feed_subscription_filter']->configuration)) {
    $form_state['#feed_subscription_filter']->configuration = array();
  }

  $form['filter'] = array(
    '#type' => 'feed_fieldset',
    '#tree' => TRUE,
  );
  $form['filter']['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Criteria Name'),
    '#description' => t('Name this subscription criteria for administrative reference.'),
    '#default_value' => $form_state['#feed_subscription_filter']->name,
    '#required' => TRUE,
  );

  $form['filter']['criteria'] = array(
    '#type' => 'feed_item_criteria',
    '#configuration' => &$form_state['#feed_subscription_filter']->configuration,
  );
  $form['filter']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  
  $form['list'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'class' => array('feed-subscription-filter-list'),
    ),
  );
  $form['list']['criterias'] = array(
  	'#type' => 'feed_table',
    '#tree' => TRUE,
    '#empty' => t('No subscription criteria created, yet. Create a new subscription criteria on the sidebar.'),
    '#header' => array(t('Name'), array('data' => t('Operation'), 'colspan' => '5')),
  );

  $lastrefresh = feed_object_get_data($form_state['#feed'], FEED_DATA_FEED_SUBSCRIBE_REFRESH_FILTER, array());
  
  foreach (feed_subscription_filter_load_all_feed($form_state['#feed']->fid) as $filter) {
    $form['list']['criterias'][$filter->fsfid] = array('#tree' => TRUE);
    $form['list']['criterias'][$filter->fsfid]['name'] = array('#markup' => $filter->name);
    $form['list']['criterias'][$filter->fsfid]['edit'] = array(
      '#type' => 'link',
      '#title' => t('Edit'),
      '#href' => "feed/subscriptions/{$feed_subscription->fid}/filter/{$filter->fsfid}",
    );
    $form['list']['criterias'][$filter->fsfid]['delete'] = array(
      '#type' => 'link',
      '#title' => t('Delete'),
      '#href' => "feed/subscriptions/{$feed_subscription->fid}/filter/{$filter->fsfid}/delete",
    );
    $form['list']['criterias'][$filter->fsfid]['run'] = array(
      '#type' => 'link',
      '#title' => t('Run'),
      '#href' => "feed/subscriptions/{$feed_subscription->fid}/filter/{$filter->fsfid}/run",
    );
    $form['list']['criterias'][$filter->fsfid]['last'] = array(
      '#type' => 'markup',
      '#markup' => t('Last ran: %last', array(
        '%last' => !empty($lastrefresh[$filter->fsfid]) ? date('n-j-y g:h:i a T', $lastrefresh[$filter->fsfid]) : 'Never'
      )),
    );
  }
  
  $form['list']['add'] = array(
    '#type' => 'submit',
    '#value' => t('Add new filter'),
    '#attributes' => array(
      'class' => array('feed-subscription-filter-add'),
    ),
  );
  return $form;
}

/**
 * Form Submit Handler for Feed subscription filter form
 */
function feed_subscription_filter_form_submit(&$form, &$form_state) {
  $feed = &$form_state['#feed'];
  $filter = &$form_state['#feed_subscription_filter'];
  
  switch ($form_state['values']['op']) {
    case $form['filter']['submit']['#value']:
      $filter->name = $form_state['values']['filter']['name'];
      $filter->configuration = $form_state['values']['filter']['criteria'];

      if (feed_subscription_filter_save($filter, $feed)) {
        unset($feed->data[FEED_DATA_FEED_SUBSCRIBE_REFRESH_FILTER][$filter->fsfid]);
        feed_save($feed);
        
        drupal_set_message(t('Feed subscription criteria successfully saved'), 'status');
      }
      else {
        drupal_set_message(t('Error saving feed subscription criteria.'), 'error');
      }
    break;
    
    case $form['list']['add']['#value']:
      drupal_goto('feed/subscriptions/' . $feed->fid . '/filter');
    break;
  }
}

/**
 * Delete feed subscription filter form;
 */
function feed_subscription_filter_delete_form($form, &$form_state, $feed, $feed_subscription_filter) {
  $form_state['#feed_subscription_filter'] = feed_subscription_filter_defaults($feed_subscription_filter);
  $form_state['#feed'] = $feed;
  
  return confirm_form(
    $form,
    t('Delete feed subscription filter?'),
    "feed/subscriptions/{$feed->fid}/filter",
    '<div class="messages warning"><span class="feed-warning">Warning:</span> ' . 
    t('You are about to delete feed subscription filter ":name". This action cannot be undone', array(':name' => $feed_subscription_filter->name)) . '</div>',
    t('Delete'),  t('Cancel')
  );
}

/**
 * Delete feed subscription filter form submit handler;
 */
function feed_subscription_filter_delete_form_submit(&$form, &$form_state) {
  $feed_subscription_filter = $form_state['#feed_subscription_filter'];
  $feed = $form_state['#feed'];
  
  if (feed_subscription_filter_delete($feed_subscription_filter)) {
    drupal_set_message(t('Feed subscription filter :name was deleted.', array(':name' => $feed_subscription_filter->name)), 'status');
  }
  else {
    drupal_set_message(t('Could not delete feed subscription filter :name.', array(':name' => $feed_subscription_filter->name)), 'error');
  }
  $form_state['redirect'] = "feed/subscriptions/{$feed->fid}/filter";
}

/**
 * Page Callback; Runs a feed filter
 */
function feed_run_subscriber_page($feed, $feed_subscription_filter) {
  if ($feed_subscription_filter->fid == $feed->fid) {
    feed_subscriber($feed, array($feed_subscription_filter));
    drupal_set_message(t('Ran filter %name', array('%name' => $feed_subscription_filter->name)), 'status');
  }
  else {
    drupal_set_message(t('Filter does not belong to this feed subscription'), 'error');
  }
  drupal_goto('feed/subscriptions/' . $feed->fid . '/filter');
}
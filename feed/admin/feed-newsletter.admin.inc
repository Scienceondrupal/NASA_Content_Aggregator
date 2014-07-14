<?php

/**
 * Form Handler; Feed newletter form;
 */
function feed_newsletter_form($form, &$form_state, $feed_newsletter = NULL) {
  $form_state['#feed_newsletter'] = feed_newsletter_defaults($feed_newsletter);

  $form['newsletter'] = array(
    '#type' => 'feed_fieldset',
    '#tree' => TRUE,
  );
  $form['newsletter']['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Newsletter Name'),
    '#description' => t('The name of this newsletter.'),
    '#default_value' => $form_state['#feed_newsletter']->name,
    '#required' => TRUE,
  );
  $form['newsletter']['criteria'] = array(
    '#type' => 'feed_item_criteria',
    '#configuration' => &$form_state['#feed_newsletter']->configuration,
  );
  $form['newsletter']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  
  $form['list'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'class' => array('feed-newsletter-list'),
    ),
  );
  $form['list']['newsletter'] = array(
    '#type' => 'feed_table',
    '#tree' => TRUE,
    '#empty' => t('No newsletter created, yet. Create a newsletter on the sidebar.'),
    '#header' => array(t('Name'), array('data' => t('Operation'), 'colspan' => '3')),
  );

  foreach (feed_newsletter_load_all_user() as $newsletter) {
    $form['list']['newsletter'][$newsletter->fnid] = array('#tree' => TRUE);
    $form['list']['newsletter'][$newsletter->fnid]['name'] = array('#markup' => $newsletter->name);
    $form['list']['newsletter'][$newsletter->fnid]['subscriber'] = array(
      '#type' => 'link',
      '#title' => t('Subscribers'),
      '#href' => "feed/newsletter/{$newsletter->fnid}/subscriber",
    );
    $form['list']['newsletter'][$newsletter->fnid]['edit'] = array(
      '#type' => 'link',
      '#title' => t('Edit'),
      '#href' => "feed/newsletter/{$newsletter->fnid}/edit",
    );
    $form['list']['newsletter'][$newsletter->fnid]['delete'] = array(
      '#type' => 'link',
      '#title' => t('Delete'),
      '#href' => "feed/newsletter/{$newsletter->fnid}/delete",
    );
  }
  return $form;
}

/**
 * Form Submit Handler; Feed newsletter submit handler;
 */
function feed_newsletter_form_submit($form, &$form_state) {
  $feed_newsletter = &$form_state['#feed_newsletter'];
  $config = $feed_newsletter->configuration;
  
  switch ($form_state['values']['op']) {
    case $form['newsletter']['submit']['#value']:
      $feed_newsletter->name = $form_state['values']['newsletter']['name'];
      $feed_newsletter->configuration = $form_state['values']['newsletter']['criteria'];

      if (feed_newsletter_save($feed_newsletter)) {
        drupal_set_message(t('Feed newsletter successfully saved'), 'status');
        
        $form_state['redirect'] = "feed/newsletter/{$feed_newsletter->fnid}";
      }
      else {
        drupal_set_message(t('Error saving feed newsletter'), 'error');
      }
    break;
  }
}

/**
 * Delete feed newsletter form;
 */
function feed_newsletter_delete_form($form, &$form_state, $feed_newsletter) {
  $form_state['#feed_newsletter'] = feed_newsletter_defaults($feed_newsletter);

  $form['#attributes'] = array('class' => array('feed-newsletter-delete-form'));
  
  return confirm_form(
    $form,
    t('Delete feed newsletter?'),
    "feed/newsletter",
    '<div class="messages warning"><span class="feed-warning">Warning:</span> ' . 
    t('You are about to delete newsletter ":name". This action cannot be undone', array(':name' => $feed_newsletter->name)) . '</div>',
    t('Delete'),  t('Cancel')
  );
}

/**
 * Delete feed newsletter form submit handler;
 */
function feed_newsletter_delete_form_submit(&$form, &$form_state) {
  $feed_newsletter = $form_state['#feed_newsletter'];
  
  if (feed_newsletter_delete($feed_newsletter)) {
    drupal_set_message(t('Feed newsletter ":name" deleted.', array(':name' => $feed_newsletter->name)), 'status');
  } 
  else {
    drupal_set_message(t('Could not delete feed newsletter ":name".', array(':name' => $feed_newsletter->name)), 'error');
  }
  $form_state['redirect'] = "feed/newsletter";
}

/**
 * Form Handler; Feed newsletter subscriber form;
 */
function feed_newsletter_subscriber_form($form, &$form_state, $feed_newsletter, $feed_newsletter_subscriber = NULL) {
  $form_state['#feed_newsletter'] = feed_newsletter_defaults($feed_newsletter);
  $form_state['#feed_newsletter_subscriber'] = feed_newsletter_subscriber_defaults($feed_newsletter_subscriber);
  
  $form['subscriber'] = array('#type' => 'feed_fieldset', '#tree' => TRUE);
  $form['subscriber']['email'] = array(
    '#type' => 'textfield',
    '#title' => t('Email Address'),
    '#element_validate' => array('element_validate_feed_email'),
    '#description' => t('The email address to send newsletter.'),
    '#default_value' => $form_state['#feed_newsletter_subscriber']->email,
    '#required' => TRUE,
  );
  $form['subscriber']['frequency'] = array(
    '#type' => 'select',
    '#title' => t('Newsletter Frequency'),
    '#default_value' => feed_object_get_configuration($form_state['#feed_newsletter_subscriber'], FEED_EMAIL_CONFIG_FREQ),
    '#required' => TRUE,
    '#options' => array(
      'daily' => 'daily',
      'biweekly' => 'biweekly',
      'weekly' => 'weekly',
      'monthly' => 'monthly',
    ),
  );
  $form['subscriber']['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save email'),
    '#attributes' => array(
      'class' => array('feed-newsletter-subscriber-save'),
    ),
  );
  
  $form['list'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'class' => array('feed-newsletter-subscriber-list'),
    ),
  );
  $form['list']['subscribers'] = array(
    '#type' => 'feed_table',
    '#tree' => TRUE,
    '#empty' => t('No feed newsletter subscribers, yet. Add a new feed newsletter subscriber on the sidebar.'),
    '#header' => array(t('Email'), t('Frequency'), array('data' => t('Operation'), 'colspan' => '4')),
  );
  foreach (feed_newsletter_subscribers_load_all($form_state['#feed_newsletter']) as $subscriber) {
    $form['list']['subscribers'][$subscriber->fnsid] = array('#tree' => TRUE);
    $form['list']['subscribers'][$subscriber->fnsid]['email'] = array('#markup' => $subscriber->email);
    $form['list']['subscribers'][$subscriber->fnsid]['frequency'] = array(
      '#markup' => feed_object_get_configuration($subscriber, FEED_EMAIL_CONFIG_FREQ, '&nbsp;'),
    );
    $lastrefresh = feed_object_get_configuration($subscriber, FEED_EMAIL_CONFIG_REFRESH);
    $form['list']['subscribers'][$subscriber->fnsid]['last'] = array(
      '#markup' => t('Last sent: %last', array(
        '%last' => !empty($lastrefresh) && is_integer($lastrefresh) ? date('n-j-y', $lastrefresh) : 'Never',
      )),
    );
    $form['list']['subscribers'][$subscriber->fnsid]['send'] = array(
      '#type' => 'link',
      '#title' => t('Send'),
      '#href' => "feed/newsletter/{$feed_newsletter->fnid}/subscriber/{$subscriber->fnsid}/run",
    );
    $form['list']['subscribers'][$subscriber->fnsid]['edit'] = array(
      '#type' => 'link',
      '#title' => t('Edit'),
      '#href' => "feed/newsletter/{$feed_newsletter->fnid}/subscriber/{$subscriber->fnsid}/edit",
    );
    $form['list']['subscribers'][$subscriber->fnsid]['delete'] = array(
      '#type' => 'link',
      '#title' => t('Delete'),
      '#href' => "feed/newsletter/{$feed_newsletter->fnid}/subscriber/{$subscriber->fnsid}/delete",
    );
  }
  $form['list']['add'] = array(
    '#type' => 'submit',
    '#value' => t('Add new subscriber'),
    '#attributes' => array(
      'class' => array('feed-newsletter-subscriber-add'),
    ),
  );
  return $form;
}

/**
 * Form Submit Handler; Feed newsletter subscriber submit handler;
 */
function feed_newsletter_subscriber_form_submit($form, &$form_state) {
  $feed_newsletter = &$form_state['#feed_newsletter'];
  $feed_newsletter_subscriber = &$form_state['#feed_newsletter_subscriber'];
  
  switch ($form_state['values']['op']) {
    case $form['subscriber']['save']['#value']:
      $feed_newsletter_subscriber->email = $form_state['values']['subscriber']['email'];
      
      feed_object_set_configuration($feed_newsletter_subscriber, FEED_EMAIL_CONFIG_FREQ, $form_state['values']['subscriber']['frequency']);

      if (feed_newsletter_subscriber_save($feed_newsletter_subscriber, $feed_newsletter)) {
        drupal_set_message(t('The email ":email" was successfully subscribed to newsletter ":name"', array(
          ':name' => $feed_newsletter->name, ':email' => $feed_newsletter_subscriber->email,
        )), 'status');
      }
      else {
        drupal_set_message(t('Error subscribing email ":email" to newsletter ":name"', array(
          ':name' => $feed_newsletter->name, ':email' => $feed_newsletter_subscriber->email,
        )), 'error');
      }
    break;
  }
}

/**
 * Element validator; Validate email address
 */
function element_validate_feed_email($element, &$form_state, $form) {
   if (!valid_email_address($element['#value'])) {
     form_error($element, t('A valid email address is required.'));
   }
}

/**
 * Delete feed newsletter subscriber form;
 */
function feed_newsletter_subscriber_delete_form($form, &$form_state, $feed_newsletter, $feed_newsletter_subscriber) {
  $form_state['#feed_newsletter'] = $feed_newsletter;
  $form_state['#feed_newsletter_subscriber'] = feed_newsletter_subscriber_defaults($feed_newsletter_subscriber);

  $form['#attributes'] = array('class' => array('feed-newsletter-subscriber-delete-form'));
  
  return confirm_form(
    $form,
    t('Delete feed newsletter subscriber?'),
    "feed/newsletter/{$feed_newsletter->fnid}/subscriber",
    '<div class="messages warning"><span class="feed-warning">Warning:</span> ' . 
    t('You are about to delete an email subscriber ":email". This action cannot be undone', array(':email' => $feed_newsletter_subscriber->email)) . '</div>',
    t('Delete'),  t('Cancel')
  );
}

/**
 * Delete feed newsletter subscriber form submit handler;
 */
function feed_newsletter_subscriber_delete_form_submit(&$form, &$form_state) {
  $feed_newsletter = $form_state['#feed_newsletter'];
  $feed_newsletter_subscriber = $form_state['#feed_newsletter_subscriber'];
  
  if (feed_newsletter_subscriber_delete($feed_newsletter_subscriber)) {
    drupal_set_message(t('Email subscriber ":email" for newsletter ":name" unsubscribed.', array(
      ':email' => $feed_newsletter_subscriber->email, 
      ':name' => $feed_newsletter->name,
    )), 'status');
  }
  else {
    drupal_set_message(t('Could not unsubscribe email ":email" for newsletter ":name".', array(
      ':email' => $feed_newsletter_subscriber->email, 
      ':name' => $feed_newsletter->name,
    )), 'status');
  }
  $form_state['redirect'] = "feed/newsletter/{$feed_newsletter->fnid}/subscriber";
}

/**
 * Page Callback; Runs a feed newsletter subscriber
 */
function feed_run_newsletter_subscriber_page($feed_newsletter, $feed_newsletter_subscriber) {
  if ($feed_newsletter_subscriber->fnid == $feed_newsletter->fnid) {
    if (feed_emailer($feed_newsletter_subscriber)) {
      drupal_set_message(t('Sent newsletter "%name" to email "%email"', array(
        '%name' => $feed_newsletter->name, 
        '%email' => $feed_newsletter_subscriber->email,
      )), 'status');
    }
    else {
      drupal_set_message(t('Error sending newsletter "%name" to email "%email"', array(
        '%name' => $feed_newsletter->name, 
        '%email' => $feed_newsletter_subscriber->email,
      )), 'error');
    }
  }
  else {
    drupal_set_message(t('Email is not subscribed to this newsletter'), 'error');
  }
  drupal_goto('feed/newsletter/' . $feed_newsletter->fnid . '/subscriber');
}
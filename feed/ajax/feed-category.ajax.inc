<?php

/**
 * AJAX Page Callback; Returns results from autocomplete request.
 */
function feed_category_term_autocomplete_ajax() {
  $search = trim($_GET['query']);
  
  $results = array('new' => '<div class="feed-category-ac-new">' . t('Add as New Tag') . '</div>');
  if (!empty($search)) {
    foreach (feed_category_term_autocomplete($search) as $term) {
      $title = t('%category: %term', array('%category' => $term->category, '%term' => $term->name));
      
      $results[$term->fctid] = array();
      $results[$term->fctid]['label'] = '<div class="feed-category-ac-item">' . $title . '</div>';
      $results[$term->fctid]['category'] = $term->category;
      $results[$term->fctid]['term'] = $term->name;
    }
  }
  drupal_json_output(array('data' => $results));
}
<div class="feed-formatter-web-container">
  <?php if (!empty($element['feed_fields'])): ?>
  <div class="feed-formatter-web-feed-field-container">
    <?php foreach ($element['feed_fields'] as $feed_field): ?>
    <div class="feed-formatter-web-feed-field">
      <div class="feed-formatter-web-feed-field-title"><?php print $feed_field['#title'] ?>:</div>
      <?php foreach ($feed_field['feed_field_values'] as $feed_field_value): ?>
        <?php if (!is_array($feed_field_value)): ?>
        <div class="feed-formatter-web-feed-field-content"><?php print $feed_field_value ?></div>
        <?php elseif (!empty($feed_field_value['#content'])): ?>
        <div class="feed-formatter-web-feed-field-content"><?php print $feed_field_value['#content'] ?></div>
        <?php endif ?>
      <?php endforeach ?>
    </div>
    <?php endforeach ?>
  </div>
  <?php endif ?>
  
  <div class="feed-formatter-web-feed-filter-options">
    <?php if (empty($element['params']['random']) && isset($element['pagination']['page_start'], $element['pagination']['page_end']) && $element['pagination']['page_end'] > $element['pagination']['page_start']): ?>
    <ul class="feed-formatter-web-feed-pagination">
      <li>Page:</li>
      <?php if (isset($element['pagination']['page_start']) && $element['pagination']['page_start'] != 1): ?>
        <li><?php print l('...', $_GET['q'], array('query' => array_merge($element['params'], array('page' => $element['pagination']['page_start'])), 'attributes' => array('class' => array('feed-formatter-web-feed-pagination-first')))) ?></li>
      <?php endif ?>
      
      <?php for ($page = $element['pagination']['page_start']; $page <= $element['pagination']['page_end']; $page++): ?>
        <?php if ($page != $element['pagination']['page_num']): ?>
        <li><?php print l($page, $_GET['q'], array('query' => array_merge($element['params'], array('page' => $page)), 'attributes' => array('class' => array('feed-formatter-web-feed-pagination-page')))) ?></li>
        <?php else: ?>
        <li><span class="feed-formatter-web-feed-pagination-current"><?php print $page ?></span></li>
        <?php endif ?>
      <?php endfor; ?>
      
      <?php if (isset($element['pagination']['page_max'], $element['pagination']['page_end']) && $element['pagination']['page_end'] < $element['pagination']['page_max']): ?>
        <li><?php print l('...', $_GET['q'], array('query' => array_merge($element['params'], array('page' => $element['pagination']['page_end'])), 'attributes' => array('class' => array('feed-formatter-web-feed-pagination-last')))) ?></li>
      <?php endif ?>
    </ul>
    <?php endif ?>
  </div>
  
  <?php if (!empty($element['feed_items'])): ?>
  <div class="feed-formatter-web-feed-item-container">
    <?php foreach ($element['feed_items'] as $feed_item): ?>
      <?php print $feed_item ?>
    <?php endforeach ?>
  </div>
  <?php endif ?>
</div>
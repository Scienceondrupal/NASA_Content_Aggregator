<div class="feed-formatter-web-feed-item">
  <div class="feed-formatter-web-feed-item-field-container">
    <?php foreach ($element['feed_item_fields'] as $feed_item_field): ?>
    <div class="feed-formatter-web-feed-item-field">
      <div class="feed-formatter-web-feed-item-field-title"><?php print $feed_item_field['#title'] ?>:</div>
      <?php foreach ($feed_item_field['feed_item_field_values'] as $feed_item_field_value): ?>
        <div class="feed-formatter-web-feed-item-field-content"><?php print $feed_item_field_value ?></div>
      <?php endforeach ?>
    </div>
    <?php endforeach ?>
  </div>
</div>
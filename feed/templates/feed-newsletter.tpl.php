<style type="text/css">
  .feed-newsletter {
    font-family: arial, sans-serif;
  }
  .feed-newsletter-title {
    color: #164EA8;
    font-weight: bold;
    font-size: 16px;
    padding-bottom: 10px;
    margin-top: 10px;
  }
  .feed-newsletter-feed {
    padding: 10px;
    margin-bottom: 10px;
    border: 1px solid #ddd;
  }
  .feed-newsletter-feed-title {
    color: #164EA8;
    font-weight: bold;
    font-size: 15px;
    margin-bottom: 10px;
  }
  .feed-newsletter-feed-items {
    border-top: 1px dotted #99B1D9;
    padding-top: 10px;
    margin-top: 10px;
  }
  .feed-newsletter-feed-item {
    margin-bottom: 10px;
    padding-bottom: 10px;
    border-bottom: 1px dotted #99B1D9;
  }
  .feed-newsletter-feed-item-title {
    color: #164EA8;
    font-weight: bold;
  }
  .feed-newsletter-feed-item-date {
    font-size: 10px;
  }
  .feed-newsletter-feed-item-description {
    margin-top: 5px;
  }
</style>

<div class="feed-newsletter">
  <div class="feed-newsletter-title">Newsletter: <?php print $variables['name']; ?></div>
  
  <div class="feed-newsletter-feeds">
    <?php foreach ($variables['feeds'] as $feed): ?>
    <div class="feed-newsletter-feed">
      <div class="feed-newsletter-feed-title">
        <?php if (!empty($feed['link'])): ?>
          <a href="<?php print $feed['link']; ?>" title="Feed Permalink">Feed: <?php print $feed['title']; ?></a>
        <?php else: ?>
          Feed: <?php print $feed['title']; ?>
        <?php endif; ?>
      </div>
      <?php if (!empty($feed['description'])): ?>
        <div class="feed-newsletter-feed-description"><?php print $feed['description']; ?></div>
      <?php endif; ?>
      <div class="feed-newsletter-feed-items">
        <?php foreach($feed['items'] as $item): ?>
        <div class="feed-newsletter-feed-item">
          <div class="feed-newsletter-feed-item-title">
            <?php if (!empty($item['link'])): ?>
              <a href="<?php print $item['link']; ?>" title="Feed Item Permalink"><?php print $item['title']; ?></a>
            <?php else: ?>
              <?php print $item['title']; ?>
            <?php endif; ?>
          </div>
          <div class="feed-newsletter-feed-item-date"><?php print $item['date']; ?></div>
          <?php if (!empty($item['description'])): ?>
            <div class="feed-newsletter-feed-item-description"><?php print $item['description']; ?></div>
          <?php endif; ?>
        </div>
        <?php endforeach; ?>
      </div>
    </div>
    <?php endforeach; ?>
  </div>
</div>

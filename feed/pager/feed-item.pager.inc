<?php 

/**
 * Feed Item Pager class
 */
class FeedItemPager {
  
  protected $queryFeedItem;
  protected $queryFeedItemField;
  protected $queryFeedItemFieldValue;
  protected $queryFeedItemFieldValueData;
  protected $queryFeedGroup;
  protected $queryFeedCategoryTerm;
  protected $queryFeedItemCategoryTerm;
  
  protected $fieldValueSubqueries = array();
  
  protected $alterMetaData;
  protected $alterTags;
  
  protected $limit;
  protected $page;
  
  public function __construct($limit = 20) {
    $this->setItemLimit($limit);
  }
  
  public function addTag($tag) {
    $this->alterTags[$tag] = 1;
    return $this;
  }

  public function hasTag($tag) {
    return isset($this->alterTags[$tag]);
  }

  public function hasAllTags() {
    $args = func_get_args();
    return !(boolean) array_diff($args, array_keys($this->alterTags));
  }

  public function hasAnyTag() {
    $args = func_get_args();
    return (boolean) array_intersect($args, array_keys($this->alterTags));
  }

  public function addMetaData($key, $object) {
    $this->alterMetaData[$key] = $object;
    return $this;
  }

  public function getMetaData($key) {
    return isset($this->alterMetaData[$key]) ? $this->alterMetaData[$key] : NULL;
  }
  
  /**
   * Gets the feed item limit.
   */
  public function getItemLimit() {
    return !empty($this->limit) ? $this->limit : 20;
  }
  
  /**
   * Sets the feed item limit.
   */
  public function setItemLimit($limit) {
    if ($limit > 0) {
      $this->limit = $limit;
    }
  }
  
  /**
   * Sets the page number of feed item query.
   */
  public function setCurrentPage($page) {
    if ($page > 0) {
      $this->page = $page;
    }
  }
  
  /**
   * Gets the page number of feed item query.
   */
  public function getCurrentPage() {
    return !empty($this->page) ? $this->page : 1;
  }
  
  /**
   * Get the max page.
   */
  public function getMaxPage() {
    return ceil($this->count() / $this->getItemLimit());
  }
  
  /**
   * Returns the feed item query object.
   */
  public function getFeedItemQuery() {
    if (!$this->queryFeedItem) {
      $this->queryFeedItem = db_select('feed_item', 'fi')->fields('fi');
    }
    return $this->queryFeedItem;
  }

  /**
   * Returns the feed group query object.
   */
  public function getFeedGroupQuery() {
    if (!$this->queryFeedGroup) {
      $this->queryFeedGroup = db_select('feed_group', 'fg')->fields('fg')->fields('f', array('fid'));
      $this->queryFeedGroup->join('feed_user_group', 'fug', 'fug.fgid = fg.fgid AND fug.primary_group = 1');
      $this->queryFeedGroup->join('feed', 'f', 'f.uid = fug.uid');
    }
    return $this->queryFeedGroup;
  }

  /**
   * Returns the feed category term query object.
   */
  public function getFeedCategoryTermQuery() {
    if (!$this->queryFeedCategoryTerm) {
      $this->queryFeedCategoryTerm = db_select('feed_category_term', 'fct')->fields('fct');
      $this->queryFeedCategoryTerm->join('feed_category_term_index', 'fcti', 'fcti.fctid = fct.fctid');
      $this->queryFeedCategoryTerm->join('feed_category', 'fc', 'fc.fcid = fct.fcid');
    }
    return $this->queryFeedCategoryTerm;
  }

  /**
   * Returns the feed item category term query object.
   */
  public function getFeedItemCategoryTermQuery() {
    if (!$this->queryFeedItemCategoryTerm) {
      $this->queryFeedItemCategoryTerm = db_select('feed_category_term', 'fct')->fields('fct');
      $this->queryFeedItemCategoryTerm->join('feed_item_category_term_index', 'ficti', 'ficti.fctid = fct.fctid');
      $this->queryFeedItemCategoryTerm->join('feed_category', 'fc', 'fc.fcid = fct.fcid');
      $this->queryFeedItemCategoryTerm->fields('ficti', array('fiid'));
    }
    return $this->queryFeedItemCategoryTerm;
  }
  
  /**
   * Returns a new feed field value subquery instance.
   * 
   * @param $name
   *   A name for feed item field value subquery instance.
   */
  public function getFeedItemFieldValueSubquery($name) {
    if (!isset($this->fieldValueSubqueries[$name])) {
      $this->fieldValueSubqueries[$name] = db_select('feed_item_field_value', 'fifv');
      $this->fieldValueSubqueries[$name]->fields('fifv', array('fiid'));
      $this->fieldValueSubqueries[$name]->distinct(TRUE);
      $this->fieldValueSubqueries[$name]->join('feed_field', 'ff', 'ff.ffid = fifv.ffid');
      $this->fieldValueSubqueries[$name]->join('feed_item_field_value_data', 'fifvd', 'fifvd.fifvid = fifv.fifvid');
    }
    return $this->fieldValueSubqueries[$name];
  }

  /**
   * Finds only feed items belonging to a feed group id.
   */
  public function hasFeedGroupId($fgid) {
    if ($this->getFeedGroupQuery()) {
      $this->getFeedItemQuery()->condition('fg.fgid', $fgid);
    }
    return $this;
  }
  
  /**
   * Finds only feed items belonging to a feed group name.
   */
  public function hasFeedGroup($name) {
    if ($this->getFeedGroupQuery()) {
      $this->getFeedItemQuery()->condition('fg.name', $name);
    }
    return $this;
  }
  
  /**
   * Finds only feed items subscribed by feed
   */
  public function hasFeedSubscribed($fid) {
    $this->addTag('join_feed_subscription');
    $this->addMetaData('has_feed_subscribed', $fid);
    return $this;
  }
  
  /**
   * Finds only feed items belonging to feed
   */
  public function hasFeedCreated($fid) {
    $this->addMetaData('has_feed_created', $fid);
    return $this;
  }
  
  /**
   * Finds only feed items with tag_id(s).
   * 
   * @param $fctid
   *   A feed category term id or array of feed category term ids.
   */
  public function hasFeedItemTagIds($fctid) {
    if (is_array($fctid)) {
      $this->getFeedItemCategoryTermQuery()->condition('fct.fctid', $fctid, 'IN');
    }
    else {
      $this->getFeedItemCategoryTermQuery()->condition('fct.fctid', $fctid);
    }
    return $this;
  }
  
  /**
   * Finds only feed items that does not have tag_id(s).
   * 
   * @param $fctid
   *   A feed category term id or array of feed category term ids.
   */
  public function doesNotHaveFeedItemTagIds($fctid) {
    if (is_array($fctid)) {
      $this->getFeedItemCategoryTermQuery()->condition('fct.fctid', $fctid, 'NOT IN');
    }
    else {
      $this->getFeedItemCategoryTermQuery()->condition('fct.fctid', $fctid, '<>');
    }
    return $this;
  }
  
  /**
   * Finds only feed items with tag(s).
   */
  public function hasFeedItemTagNames($tag) {
    if (is_array($tag)) {
      $this->getFeedItemCategoryTermQuery()->condition('fct.name', $tag, 'IN');
    }
    else {
      $this->getFeedItemCategoryTermQuery()->condition('fct.name', $tag);
    }
    return $this;
  }
  
  /**
   * Finds only feed items that does not have tag(s).
   */
  public function doesNotHaveFeedItemTagNames($tag) {
    if (is_array($tag)) {
      $this->getFeedItemCategoryTermQuery()->condition('fct.name', $tag, 'NOT IN');
    }
    else {
      $this->getFeedItemCategoryTermQuery()->condition('fct.name', $tag, '<>');
    }
    return $this;
  }

  /**
   * Finds only feed items by tagname, field data, and field value
   * 
   * @param $tagname
   *   The feed item field tagname.
   * @param $data
   *   Array of data name and data value to search.
   * @param $operator
   *   The conditional operator.
   */
  public function hasFieldValueByTagname($tagname, array $datas, $operator = NULL) {
    if (strpos($tagname, ':') !== FALSE) {
      list($namespace, $tag) = explode(':', $tagname, 2);
    }
    else {
      $tag = $tagname;
    }
    
    $field_condition = db_and();
    $field_condition->condition('ff.tag', $tag);
    if (!empty($namespace)) {
      $field_condition->condition('ff.namespace', $namespace);
    }
    foreach ($datas as $name => $data) {
      $field_condition->condition('fifvd.name', $name);
      $field_condition->condition('fifvd.data', $data, $operator);
    }
    $this->getFeedItemFieldValueSubquery($tagname)->condition($field_condition);
    
    return $this;
  }
  
  /**
   * Finds only feed itmes by tagname, field condition
   * 
   * @param $tagname
   *   The feed item field tagname.
   * @param $condition
   *   Instance of QueryConditionInterface as returned by db_and(), db_or, and db_condition().
   */
  public function hasFieldConditionByTagname($tagname, QueryConditionInterface $condition) {
    if (strpos($tagname, ':') !== FALSE) {
      list($namespace, $tag) = explode(':', $tagname, 2);
    }
    else {
      $tag = $tagname;
    }
    
    $field_condition = db_and();
    $field_condition->condition('ff.tag', $tag);
    if (!empty($namespace)) {
      $field_condition->condition('ff.namespace', $namespace);
    }
    $this->getFeedItemFieldValueSubquery($tagname)
      ->condition($field_condition)
      ->condition($condition);
    
    return $this;
  }
  
  /**
   * Adds a field value column to the query by field tagname
   * 
   * @param $tagname
   *   The feed item tagname.
   * @param $dataname
   *   The feed item field data name;
   * @param $sort
   *   The sorting criteria
   * @param $separator
   *   The separator
   */
  public function addFieldValueColumnByTagname($tagname, $dataname, $alias, $sort = 'ASC', $separator = ',') {
    if (!preg_match('/^[A-Za-z0-9_]+$/', $alias)) {
      throw new Exception('FeedItemPager::addFieldValueColumnByTagname $alias must contain alphanumerica and underscore characters only.');
    }
    $tag = $tagname;
    if (strpos($tagname, ':') !== FALSE) {
      list($namespace, $tag) = explode(':', $tagname, 2);
    }
    
    $expression = 'GROUP_CONCAT(IF(ff.tag = ' . ":{$alias}_tag";
    $arguments = array(":{$alias}_tag" => $tag);
    
    if (!empty($namespace)) {
      $expression .= ' AND ff.namespace = ' . ":{$alias}_namespace";
      $arguments[":{$alias}_namespace"] = $namespace;
    }
    $expression .= ' AND fifvd.name = ' . ":{$alias}_dataname";
    $arguments[":{$alias}_dataname"] = $dataname;
      
    $expression .= ', fifvd.data, NULL)';
    if (in_array($sort, array('DESC', 'ASC'))) {
    	$expression .= ' ORDER BY fifvd.data ' . $sort;
    }
    $expression .= ' SEPARATOR ' . ":{$alias}_separator" . ')';
    $arguments[":{$alias}_separator"] = $separator;
      
    $this->getFeedItemQuery()->addExpression($expression, $alias, $arguments);
    
    return $this;
  }
  
  /**
   * Builds the joins. We delay this until final query to prevent
   * unnecessary joins.
   */
  public function buildQuery() {
    $feed_item_query = clone $this->getFeedItemQuery();
    foreach ($this->fieldValueSubqueries as $alias => $fieldValueSubquery) {
      $feed_item_query->join(clone $fieldValueSubquery, $alias, "{$alias}.fiid = fi.fiid");
    }
    $feed_item_query->join('feed_item_field_value', 'fifv', 'fifv.fiid = fi.fiid');
    $feed_item_query->join('feed_field', 'ff', 'fifv.ffid = ff.ffid');
    $feed_item_query->join('feed_item_field_value_data', 'fifvd', 'fifvd.fifvid = fifv.fifvid');
    $feed_item_query->groupBy('fi.fiid');
    
    if ($this->queryFeedGroup) {
      $feed_item_query->leftJoin(clone $this->queryFeedGroup, 'fg', 'fg.fid = fi.fid');
    }
    
    if ($this->queryFeedItemCategoryTerm) {
      $feed_item_query->join(clone $this->queryFeedItemCategoryTerm, 'fict', 'fict.fiid = fi.fiid');
    }
    
    if ($this->hasTag('join_feed_subscription')) {
      $feed_item_query->leftJoin('feed_item_subscription', 'fis', 'fis.fiid = fi.fiid');
    }
    
    $condition = db_or();
    if ($fid = $this->getMetaData('has_feed_subscribed')) {
      $condition->condition('fis.fid', $fid);
    }
    
    if ($fid = $this->getMetaData('has_feed_created')) {
      $condition->condition('fi.fid', $fid);
    }
    
    if ($condition->count()) {
      $feed_item_query->condition($condition);
    }
    return $feed_item_query;
  }
  
  /**
   * Creates a clone of the query.
   */
  public function build() {
    $query = $this->buildQuery();
    $query->range(($this->getCurrentPage() - 1) * $this->getItemLimit(), $this->getItemLimit());
    
    return $query;
  }
  
  /**
   * Performs the count
   */
  public function count() {
    return $this->buildQuery()->countQuery()->execute()->fetchColumn();
  }
  
  /**
   * Performs the query
   */
  public function query() {
    $results = $this->build()->execute()->fetchAllAssoc('fiid');
    
    if (!empty($results)) {
      drupal_alter('feed_items_load', $results);
    }
    return $results;
  }
}

/**
SELECT
  fi.*,
  GROUP_CONCAT(IF(ff.tag = 'title', fifvd.data, NULL) ORDER BY ff.tag ASC SEPARATOR ',') AS fieldtagname_value_1,
  GROUP_CONCAT(IF(ff.tag = 'description', fifvd.data, NULL) ORDER BY ff.tag ASC SEPARATOR ',') AS fieldtagname_value_2
FROM
  feed_item fi
  
RIGHT JOIN (
  SELECT 
    DISTINCT fifv.fiid
  FROM 
    feed_item_field_value fifv
  RIGHT JOIN
    feed_field ff ON (ff.ffid = fifv.ffid)
  RIGHT JOIN
    feed_item_field_value_data fifvd ON (fifvd.fifvid = fifv.fifvid)
  WHERE
    (ff.tag = 'title')
    AND
    (
      (fifvd.name = 'text') AND
      (fifvd.data LIKE '%GRACE%' ESCAPE '\\')
    )
) AS title ON (title.fiid = fi.fiid)

RIGHT JOIN (
  SELECT 
    DISTINCT fifv.fiid
  FROM 
    feed_item_field_value fifv
  RIGHT JOIN
    feed_field ff ON (ff.ffid = fifv.ffid)
  RIGHT JOIN
    feed_item_field_value_data fifvd ON (fifvd.fifvid = fifv.fifvid)
  WHERE
    (ff.tag = 'description')
    AND
    (
      (fifvd.name = 'html') AND
      (fifvd.data LIKE '%NASA%' ESCAPE '\\')
    )
) AS description ON (description.fiid = fi.fiid)

INNER JOIN feed_item_field_value fifv ON (fifv.fiid = fi.fiid)
INNER JOIN feed_field ff ON (fifv.ffid = ff.ffid)
INNER JOIN feed_item_field_value_data fifvd ON (fifvd.fifvid = fifv.fifvid)

GROUP BY fi.fiid
 */
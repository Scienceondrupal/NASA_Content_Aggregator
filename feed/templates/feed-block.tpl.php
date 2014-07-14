<?php 

/**
 * @file
 * Default theme implementation to display a feed block.
 * 
 * Available variables:
 * 	- $title: the feed block title.
 *  - $content: the feed block content, as a renderable drupal array or content string.
 * 	- $attributes: the feed block attributes.
 **/
?>
<div<?php print !empty($classes) ? ' class="' . $classes . '"' : NULL  ?><?php print $attributes ?>>
  <?php if (!empty($title)): ?>
	<div<?php print $title_attributes ?>>
		<?php print render($title) ?>
	</div>
  <?php endif ?>
  
  <?php if (!empty($content)): ?>
  <div<?php print $content_attributes ?>>
	  <?php print render($content) ?>
	</div>
	<?php endif ?>
</div>
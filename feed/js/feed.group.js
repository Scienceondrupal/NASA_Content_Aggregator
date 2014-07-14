(function($) {
  Drupal.behaviors.feedGroupUserDrag = {
    attach: function(context, settings) {
      if (typeof Drupal.tableDrag == 'undefined' || typeof Drupal.tableDrag.feedGroupUserDrag == 'undefined') {
        return;
      }
      
      var table = $('table#feedGroupUserDrag');
      var tableDrag = Drupal.tableDrag.feedGroupUserDrag;
      
      tableDrag.row.prototype.onSwap = function (swappedRow) {
        checkEmptyGroups(table, this);
      };
      
      // A custom message for the blocks page specifically.
      Drupal.theme.tableDragChangedWarning = function () {
        var message = $('.feed-message');
        
        if (message.length) {
          message.html('<div class="messages warning">' + Drupal.theme('tableDragChangedMarker') + ' ' + Drupal.t('The changes to these groups will not be saved until the <em>Save</em> button is clicked.') + '</div>');
        }
      };

      // Add a handler so when a row is dropped, update fields dropped into new groups.
      tableDrag.onDrop = function () {
        dragObject = this;
        // Use "group-message" row instead of "group" row because
        // "group-{group_name}-message" is less prone to regexp match errors.
        var groupRow = $(dragObject.rowObject.element).prevAll('tr.group-message').get(0);
        var groupName = groupRow.className.replace(/([^ ]+[ ]+)*group-([^ ]+)-message([ ]+[^ ]+)*/, '$2');
        var groupField = $('select.feed-group-select', dragObject.rowObject.element);
        
        // Check whether the newly picked group is available for this user.
        if ($('option[value=' + groupName + ']', groupField).length == 0) {
          // If not, alert the user and keep the block in its old group setting.
          alert(Drupal.t('The user cannot be placed in this group.'));
          // Simulate that there was a selected element change, so the row is put
          // back to from where the user tried to drag it.
          groupField.change();
        }
        else if ($(dragObject.rowObject.element).prev('tr').is('.group-message')) {
          // var weightField = $('select.block-weight', dragObject.rowObject.element);
          var oldGroupName = groupField.find('option:selected').val();

          if (!groupField.is('.feed-group-' + groupName)) {
            groupField.removeClass('feed-group-' + oldGroupName).addClass('feed-group-' + groupName);
            //weightField.removeClass('block-weight-' + oldGroupName).addClass('block-weight-' + groupName);
            groupField.val(groupName);
          }
        }
      };
      
      // Add the behavior to each group select list.
      $('select.block-group-select', context).once('block-group-select', function () {
        $(this).change(function (event) {
          // Make our new row and select field.
          var row = $(this).parents('tr:first');
          var select = $(this);
          tableDrag.rowObject = new tableDrag.row(row);

          // Find the correct group and insert the row as the first in the group.
          $('tr.group-message', table).each(function () {
            if ($(this).is('.group-' + select[0].value + '-message')) {
              // Add the new row and remove the old one.
              $(this).after(row);
              // Manually update weights and restripe.
              tableDrag.updateFields(row.get(0));
              tableDrag.rowObject.changed = true;
              if (tableDrag.oldRowElement) {
                $(tableDrag.oldRowElement).removeClass('drag-previous');
              }
              tableDrag.oldRowElement = row.get(0);
              tableDrag.restripeTable();
              tableDrag.rowObject.markChanged();
              tableDrag.oldRowElement = row;
              $(row).addClass('drag-previous');
            }
          });

          // Modify empty groups with added or removed fields.
          checkEmptyGroups(table, row);
          // Remove focus from selectbox.
          select.get(0).blur();
        });
      });
      
      var checkEmptyGroups = function (tbale, rowObject) {
        $('tr.group-message', table).each(function () {
          // If the dragged row is in this group, but above the message row, swap it down one space.
          if ($(this).prev('tr').get(0) == rowObject.element) {
            // Prevent a recursion problem when using the keyboard to move rows up.
            if ((rowObject.method != 'keyboard' || rowObject.direction == 'down')) {
              rowObject.swap('after', this);
            }
          }
          // This group has become empty.
          if ($(this).next('tr').is(':not(.draggable)') || $(this).next('tr').size() == 0) {
            $(this).removeClass('group-populated').addClass('group-empty');
          }
          // This group has become populated.
          else if ($(this).is('.group-empty')) {
            $(this).removeClass('group-empty').addClass('group-populated');
          }
        });
      }
    }
  }
})(jQuery);

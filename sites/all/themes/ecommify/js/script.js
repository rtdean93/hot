(function($) {
  Drupal.behaviors.ecommify = {
    attach: function(context, settings) {
      var search_sel = "#header #navigation #block-views-exp-seach-results-page .form-item input",
          search = $(search_sel).val();
      if(search != '') {
        $(search_sel).trigger('focus');
      }

      $(search_sel).blur(function() {
        if(search != '') {
          $(search_sel).trigger('focus');
        }
      });

      $(window).blur(function(){
        if(search != '') {
          $(search_sel).trigger('focus');
        }
      })
    }
  };
}) (jQuery);


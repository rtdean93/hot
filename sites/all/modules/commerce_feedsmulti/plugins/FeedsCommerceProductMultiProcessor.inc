<?php
/**
 * @file
 * Class definition of FeedsCommerceProductMultiProcessor.
 *
 * @todo Allow differentiation of identically named options in mapper (perhaps
 *   show each bundle a field's attached to?)
 *
 * @see FeedsCommerceProductProcessor.inc
 */

/**
 * Creates products of different types from feed items.
 *
 * @see FeedsCommerceProductProcessor
 */
class FeedsCommerceProductMultiProcessor extends FeedsCommerceProductProcessor {

  /**
   * Is called immediately before newEntity(): ensure the correct product type
   * is set.
   *
   * @see FeedsProcessor::map()
   */
  protected function preNewEntity(FeedsSource $source, FeedsParserResult $parser_result) {
    
    $statics =& drupal_static(__FUNCTION__);
    
    // Store the default product type as $this->config['product_type'] is
    // overwritten when each feed item's processed.
    if (!isset($statics['default_product_type'])) {
      // $this->config['product_type'] hasn't been overwritten for this first
      // time yet, so it still contains the value from the settings form.
      $statics['default_product_type'] = $this->config['product_type'];
    }
    
    // Static cache $targets as getMappingTargets() may be an expensive method.
    $sources =& $statics['sources'];
    $parser = feeds_importer($this->id)->parser;
    if (!isset($sources[$this->id])) {
      $sources[$this->id] = $parser->getMappingSources();
    }

    // Loop through the mappings to find product_type and get its value from the
    // source
    self::loadMappers();
    foreach ($this->config['mappings'] as $mapping) {
    
      if ($mapping['target'] == 'product_type') {
        // Retrieve source element's value from parser.
        if (isset($sources[$this->id][$mapping['source']]) &&
            is_array($sources[$this->id][$mapping['source']]) &&
            isset($sources[$this->id][$mapping['source']]['callback']) &&
            function_exists($sources[$this->id][$mapping['source']]['callback'])) {
          $callback = $sources[$this->id][$mapping['source']]['callback'];
          $value = $callback($source, $result, $mapping['source']);
        }
        else {
          $value = $parser->getSourceElement($source, $parser_result, $mapping['source']);
        }
        
        // If a type's in the source use that, otherwise use the default.
        if ($value) {
          $this->config['product_type'] = $value;
        }
        else {
          $this->config['product_type'] = $statics['default_product_type'];
        }
      }
    }
  }
 
  /**
   * Overrides parent::configForm().
   */
  public function configForm(&$form_state) {
    $form = parent::configForm($form_state);
    
    // Make any necessary modifications.
    $form['product_type']['#description'] = t("Select the default product type for the products to be created. If a product type isn't provided by the source this will be used. <strong>Note:</strong> Users with \"import !feed_id feeds\" permissions will be able to <strong>import</strong> products regardless of the product level permissions. Further, users with \"clear !feed_id permissions\" will be able to <strong>delete</strong> imported products regardless of their product level permissions.", array('!feed_id' => $this->id));
    $form['product_type']['#title'] = t("Default product type");
    
    return $form;
  }

  /**
   * Return available mapping targets.
   */
  public function getMappingTargets() {
    
    $targets['product_type'] = array(
      'name' => t("Product type"),
      'description' => t("The type of the product"),
      'callback' => 'commerce_feedsmulti_blank_callback',
    );
    
    $targets += parent::getMappingTargets();
    
    // Loop through every product type to get all possible mapping targets. This
    // would cause problems if targets could vary between bundles (I don't know
    // of that being possible).
    $types = commerce_product_types();
    self::loadMappers();
    foreach ($types as $type => $info) {
      feeds_alter('feeds_processor_targets', $targets, 'commerce_product', $type);
    }
    
    return $targets;
  }
  
  /**
   * Overrides parent::process(). Process the result of the parsing stage. The
   * only difference between this and FeedsProcessor::process() is the
   * additional call to $this->preNewEntity() which comes just before calling
   * $this->newItemInfo().
   *
   * @param FeedsSource $source
   *   Source information about this import.
   * @param FeedsParserResult $parser_result
   *   The result of the parsing stage.
   */
  public function process(FeedsSource $source, FeedsParserResult $parser_result) {
  
    $state = $source->state(FEEDS_PROCESS);

    while ($item = $parser_result->shiftItem()) {
      if (!($entity_id = $this->existingEntityId($source, $parser_result)) ||
           ($this->config['update_existing'] != FEEDS_SKIP_EXISTING)) {

        // Only proceed if item has actually changed.
        $hash = $this->hash($item);
        if (!empty($entity_id) && $hash == $this->getHash($entity_id)) {
          continue;
        }

        try {
          // Assemble node, map item to it, save.
          if (empty($entity_id)) {
            // Run preNewEntity to ensure we're ready to import the correct
            // type.
            $this->preNewEntity($source, $parser_result);
            $entity = $this->newEntity($source);
            $this->newItemInfo($entity, $source->feed_nid, $hash);
          }
          else {
            $entity = $this->entityLoad($source, $entity_id);
            // The feeds_item table is always updated with the info for the most recently processed entity.
            // The only carryover is the entity_id.
            $this->newItemInfo($entity, $source->feed_nid, $hash);
            $entity->feeds_item->entity_id = $entity_id;
          }
          $this->map($source, $parser_result, $entity);
          $this->entityValidate($entity);

          // Allow modules to alter the entity before saving.
          module_invoke_all('feeds_presave', $source, $entity);
          if (module_exists('rules')) {
            rules_invoke_event('feeds_import_'. $source->importer()->id, $entity);
          }

          // Enable modules to skip saving at all.
          if (empty($entity->feeds_item->skip)) {
            $this->entitySave($entity);
            // Track progress.
            if (empty($entity_id)) {
              $state->created++;
            }
            else {
              $state->updated++;
            }
          }
        }
        catch (Exception $e) {
          $state->failed++;
          drupal_set_message($e->getMessage(), 'warning');
          $message = $e->getMessage();
          $message .= '<h3>Original item</h3>';
          $message .= '<pre>' . var_export($item, TRUE) . '</pre>';
          $message .= '<h3>Entity</h3>';
          $message .= '<pre>' . var_export($entity, TRUE) . '</pre>';
          $source->log('import', $message, array(), WATCHDOG_ERROR);
        }
      }
    }

    // Set messages if we're done.
    if ($source->progressImporting() != FEEDS_BATCH_COMPLETE) {
      return;
    }
    $info = $this->entityInfo();
    $tokens = array(
      '@entity' => strtolower($info['label']),
      '@entities' => strtolower($info['label plural']),
    );
    $messages = array();
    if ($state->created) {
      $messages[] = array(
       'message' => format_plural(
          $state->created,
          'Created @number @entity.',
          'Created @number @entities.',
          array('@number' => $state->created) + $tokens
        ),
      );
    }
    if ($state->updated) {
      $messages[] = array(
       'message' => format_plural(
          $state->updated,
          'Updated @number @entity.',
          'Updated @number @entities.',
          array('@number' => $state->updated) + $tokens
        ),
      );
    }
    if ($state->failed) {
      $messages[] = array(
       'message' => format_plural(
          $state->failed,
          'Failed importing @number @entity.',
          'Failed importing @number @entities.',
          array('@number' => $state->failed) + $tokens
        ),
        'level' => WATCHDOG_ERROR,
      );
    }
    if (empty($messages)) {
      $messages[] = array(
        'message' => t('There are no new @entities.', array('@entities' => strtolower($info['label plural']))),
      );
    }
    foreach ($messages as $message) {
      drupal_set_message($message['message']);
      $source->log('import', $message['message'], array(), isset($message['level']) ? $message['level'] : WATCHDOG_INFO);
    }
  }
}

/**
 * This is used as the product type's setter callback - it's just a noop.
 */
function commerce_feedsmulti_blank_callback() {
}

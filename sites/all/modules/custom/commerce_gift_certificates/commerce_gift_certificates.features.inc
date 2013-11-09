<?php
/**
 * @file
 * commerce_gift_certificates.features.inc
 */

/**
 * Implements hook_commerce_coupon_default_types().
 */
function commerce_gift_certificates_commerce_coupon_default_types() {
  $items = array(
    'gift_certificate' => array(
      'type' => 'gift_certificate',
      'label' => 'Gift Certificate',
      'weight' => '0',
      'data' => NULL,
      'status' => '1',
      'module' => NULL,
      'rdf_mapping' => array(),
    ),
  );
  return $items;
}

/**
 * Implements hook_commerce_line_item_default_types().
 */
function commerce_gift_certificates_commerce_line_item_default_types() {
  $items = array(
    'gift_certificate' => array(
      'name' => 'Gift Certificate',
      'description' => 'A customizable product line item type.',
      'product' => TRUE,
      'add_form_submit_value' => 'Add product',
      'base' => 'commerce_product_line_item',
    ),
  );
  return $items;
}

/**
 * Implements hook_commerce_product_default_types().
 */
function commerce_gift_certificates_commerce_product_default_types() {
  $items = array(
    'gift_certificate' => array(
      'type' => 'gift_certificate',
      'name' => 'Gift Certificate',
      'description' => '',
      'help' => '',
      'revision' => '1',
    ),
  );
  return $items;
}

/**
 * Implements hook_node_info().
 */
function commerce_gift_certificates_node_info() {
  $items = array(
    'gift_certificate_display' => array(
      'name' => t('Gift Certificate Display'),
      'base' => 'node_content',
      'description' => '',
      'has_title' => '1',
      'title_label' => t('Title'),
      'help' => '',
    ),
  );
  return $items;
}

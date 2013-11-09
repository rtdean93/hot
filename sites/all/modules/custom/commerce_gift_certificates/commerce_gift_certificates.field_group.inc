<?php
/**
 * @file
 * commerce_gift_certificates.field_group.inc
 */

/**
 * Implements hook_field_group_info().
 */
function commerce_gift_certificates_field_group_info() {
  $export = array();

  $field_group = new stdClass();
  $field_group->disabled = FALSE; /* Edit this to true to make a default field_group disabled initially */
  $field_group->api_version = 1;
  $field_group->identifier = 'group_meta_tags|node|gift_certificate_display|form';
  $field_group->group_name = 'group_meta_tags';
  $field_group->entity_type = 'node';
  $field_group->bundle = 'gift_certificate_display';
  $field_group->mode = 'form';
  $field_group->parent_name = '';
  $field_group->data = array(
    'label' => 'Meta Tags',
    'weight' => '5',
    'children' => array(
      0 => 'field_meta_description',
      1 => 'field_meta_keywords',
    ),
    'format_type' => 'tab',
    'format_settings' => array(
      'formatter' => 'closed',
      'instance_settings' => array(
        'description' => '',
        'classes' => '',
        'required_fields' => 1,
      ),
    ),
  );
  $export['group_meta_tags|node|gift_certificate_display|form'] = $field_group;

  return $export;
}

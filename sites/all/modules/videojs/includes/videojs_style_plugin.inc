<?php

/**
 * @file
 * Display style plugin for Views that renders rows as a videojs.
 */

/**
 * Style plugin to display a videojs.
 */
class videojs_style_plugin extends views_plugin_style {

  // Allow some options for the style.
  function option_definition() {
    $options = parent::option_definition();
    $options['path_field'] = array('default' => NULL);
    $options['label_field'] = array('default' => NULL);
    return $options;
  }

  // Create the options form.
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    // Build a list of all fields.
    $options = array();
    foreach ($this->display->handler->get_handlers('field') as $field => $handler) {
      if ($label = $handler->label()) {
        $options[$field] = $label;
      } else {
        $options[$field] = $handler->ui_name();
      }
    }

    if (empty($options)) {
      $options[''] = t('No available fields');
    }

    $form['path_field'] = array(
      '#type' => 'select',
      '#title' => t('File path fields'),
      '#options' => $options,
      '#multiple' => TRUE,
      '#size' => 4,
      '#default_value' => $this->options['path_field'],
      '#description' => t('Select the fields that will contain a file path to an video file. If multiple fields are selected, the first one that contains a value will be used. This field will be hidden from view unless there are no other fields visible'),
      '#weight' => -5,
    );
  }

  // Ensure we have all the settings necessary to render into tabs.
  function validate() {
    $errors = parent::validate();

    // Ensure that we're using the field row style.
    if (!$this->row_plugin->uses_fields()) {
      $errors[] = t('Display "@display" uses the "@style" row style, but the videojs display style requires use of the "Fields" row style.', array('@display' => $this->display->display_title, '@style' => $this->row_plugin->definition['title']));
    }

    // Ensure that a file path field is selected.
    if (empty($this->options['path_field'])) {
      $errors[] = t('The videojs display style requires being configured so that at least one field is selected to provide a file path.');
    }

    return $errors;
  }

  // Override the render functionality.
  function render() {
    if (empty($this->row_plugin)) {
      vpr('views_plugin_style_default: Missing row plugin');
      return;
    }

    $view = $this->view;

    // Group the rows according to the grouping field, if specified.
    $sets = $this->render_grouping($this->view->result, $this->options['grouping']);
    $fields = $this->display->handler->get_handlers('field');

    $items = array();

    foreach ($sets as $title => $records) {
      foreach ($records as $row_index => $row) {
        $filepath = NULL;
        $fileid = NULL;
        foreach ($this->options['path_field'] as $field_name) {
          if (isset($row->{$fields[$field_name]->field_alias})) {
            $fileid = $row->{$fields[$field_name]->field_alias};
            break;
          }
        }

        $file = db_fetch_object(db_query("SELECT filepath FROM {files} WHERE fid = %d", $fileid));
        $filepath = $file->filepath;
        $this->view->row_index = $row_index;
        // get the file path from the file object

        $items[] = array(
          'uri' => $filepath,
          'description' => trim(strip_tags($this->row_plugin->render($row))),
        );
      }
    }
    $view->rows = array(
      '#theme' => 'videojs',
      '#items' => $items,
      '#player_id' => 'videojs-view-' . str_replace('_', '-', $view->name),
      '#attached' => videojs_add(FALSE),
    );
    return drupal_render($view->rows);
  }

  // Because we don't provide templates, but Views may complain when visiting
  // the theme information if it can't find any. Specify the name of our theme
  // function to keep it from causing any errors.
  function theme_functions() {
    return array('videojs');
  }

}

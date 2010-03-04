/*
 * Copyright 2010 Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 * Boston, MA 02111-1307, USA.
 *
 */

#include <mx/mx.h>
#include <clutter/x11/clutter-x11.h>

static gint func = 0;

static ClutterActor *
replace_deformation (ClutterActor *texture, GType type)
{
  ClutterActor *new_texture, *parent, *front_actor, *back_actor;
  CoglHandle front, back;
  gint x, y;

  parent = clutter_actor_get_parent (texture);
  mx_deform_texture_get_materials (MX_DEFORM_TEXTURE (texture),
                                   &front,
                                   &back);
  mx_deform_texture_get_actors (MX_DEFORM_TEXTURE (texture),
                                &front_actor,
                                &back_actor);
  if (front_actor)
    g_object_ref (front_actor);
  if (back_actor)
    g_object_ref (back_actor);
  mx_deform_texture_set_actors (MX_DEFORM_TEXTURE (texture), NULL, NULL);

  new_texture = g_object_new (type, NULL);
  mx_deform_texture_set_materials (MX_DEFORM_TEXTURE (new_texture),
                                   front,
                                   back);
  mx_deform_texture_set_actors (MX_DEFORM_TEXTURE (new_texture),
                                front_actor,
                                back_actor);
  if (front_actor)
    g_object_unref (front_actor);
  if (back_actor)
    g_object_unref (back_actor);

  mx_deform_texture_get_resolution (MX_DEFORM_TEXTURE (texture), &x, &y);
  mx_deform_texture_set_resolution (MX_DEFORM_TEXTURE (new_texture), x, y);

  clutter_actor_destroy (texture);
  clutter_container_add_actor (CLUTTER_CONTAINER (parent),
                               new_texture);

  return new_texture;
}

static void
completed_cb (ClutterAnimation *animation,
              ClutterActor     *texture)
{
  switch (func)
    {
    case 0:
      /* Change direction of page-turn animation */
      clutter_actor_animate (texture, CLUTTER_EASE_IN_OUT_SINE, 5000,
                             "period", 0.0,
                             "signal-after::completed", completed_cb, texture,
                             NULL);
      break;

    case 1:
      /* Replace page-turn deformation with bow-tie deformation */
      texture = replace_deformation (texture, MX_TYPE_DEFORM_BOWTIE);
      clutter_actor_animate (texture, CLUTTER_EASE_IN_OUT_SINE, 5000,
                             "period", 1.0,
                             "signal-after::completed", completed_cb, texture,
                             NULL);
      break;

    case 2:
      /* Change direction of bow-tie animation */
      clutter_actor_animate (texture, CLUTTER_EASE_IN_OUT_SINE, 5000,
                             "period", 0.0,
                             "signal-after::completed", completed_cb, texture,
                             NULL);
      break;

    case 3:
      /* Replace bow-tie deformation with cloth deformation */
      texture = replace_deformation (texture, MX_TYPE_DEFORM_CLOTH);
      g_object_set (G_OBJECT (texture), "amplitude", 0.0, NULL);
      clutter_actor_animate (texture, CLUTTER_EASE_IN_QUAD, 5000,
                             "period", 2.0,
                             "amplitude", 1.0,
                             "signal-after::completed", completed_cb, texture,
                             NULL);
      break;

    case 4:
      /* Reverse direction of cloth deformation */
      clutter_actor_animate (texture, CLUTTER_EASE_OUT_QUAD, 5000,
                             "period", 4.0,
                             "amplitude", 0.0,
                             "signal-after::completed", completed_cb, texture,
                             NULL);
      break;

    case 5:
      /* Replace cloth deformation with page-turn deformation */
      texture = replace_deformation (texture, MX_TYPE_DEFORM_PAGE_TURN);
      clutter_actor_animate (texture, CLUTTER_EASE_IN_OUT_SINE, 5000,
                             "period", 1.0,
                             "signal-after::completed", completed_cb, texture,
                             NULL);
      break;
    }

  if (++func == 6)
    func = 0;
}

int
main (int argc, char *argv[])
{
  MxApplication *app;
  gfloat width, height;
  ClutterActor *stage, *texture;
  ClutterColor stage_color = { 0xcc, 0xcc, 0xcc, 0xb0 };

#if CLUTTER_CHECK_VERSION(1,2,0)
  /* Enable argb visuals for coolness with compositors */
  clutter_x11_set_use_argb_visual (TRUE);
#endif

  app = mx_application_new (&argc, &argv, "Test deformations", 0);

  stage = (ClutterActor *)mx_application_create_window (app);

  clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_color);

  /* Set a size so we don't just get our minimum size on map */
  clutter_actor_set_size (stage, 480, 480);

  /* Create a page-turn deformation */
  texture = mx_deform_page_turn_new ();
  mx_deform_texture_set_from_files (MX_DEFORM_TEXTURE (texture),
                                    (argc > 1) ? argv[1] : NULL,
                                    (argc > 2) ? argv[2] : NULL);
  mx_deform_texture_set_actors (MX_DEFORM_TEXTURE (texture),
                                (argc < 2) ?
                                  mx_button_new_with_label ("Front face") :
                                  NULL,
                                (argc < 3) ?
                                  mx_button_new_with_label ("Back face") :
                                  NULL);

  /* Make the subdivision size a bit higher than default so it looks nicer */
  clutter_actor_get_preferred_size (texture, NULL, NULL, &width, &height);
  mx_deform_texture_set_resolution (MX_DEFORM_TEXTURE (texture), 64, 64);

  /* Add it to the stage */
  clutter_container_add (CLUTTER_CONTAINER (stage), texture, NULL);

  /* Start animation */
  clutter_actor_animate (texture, CLUTTER_EASE_IN_OUT_SINE, 5000,
                         "period", 1.0,
                         "signal-after::completed", completed_cb, texture,
                         NULL);

  /* Begin */
  clutter_actor_show (stage);
  mx_application_run (app);

  return 0;
}

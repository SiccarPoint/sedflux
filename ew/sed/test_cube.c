#include <glib.h>
#include <sed_sedflux.h>
#include <sed_cube.h>

void
test_sed_cube_new (void)
{
  {
    Sed_cube c = sed_cube_new( 2 , 5 );

    g_assert (c!=NULL);

    g_assert_cmpint (sed_cube_n_x (c), ==, 2);
    g_assert_cmpint (sed_cube_n_y (c), ==, 5);
    g_assert ( eh_compare_dbl (sed_cube_mass (c), 0., 1e-12) );

    sed_cube_destroy( c );
  }
}

void
test_sed_cube_destroy (void)
{
   Sed_cube c = sed_cube_new( 5 , 2 );

   c = sed_cube_destroy(c);

   g_assert (c==NULL);
}

gchar* test_seqfile[] =
{
   " # Begin the first record\n" ,
   "[ TiMe: 1 ] /* Label is case insensitive*/\n" ,
   "0, -1\n" ,
   "10, 1\n" ,
   "/* The second record.\n" ,
   "*/\n" ,
   "[ time : 3 ]\n" ,
   "0, -1\n" ,
   "5, -2\n" ,
   "9, 1// file ending without an empty line" ,
   NULL
};

void
test_sequence_2 (void)
{
  GError* error = NULL;
  gchar* name_used = NULL;
  gchar* tmpdir = NULL;
  int fd = -1;
  Eh_sequence* s = NULL;
  gint n_y = 10;
  double* y = eh_linspace( 0 , 9 , n_y );

  //fd = g_file_open_tmp ("sed_cube_test_XXXXXX", &name_used, &error);
  tmpdir = g_build_filename (g_get_tmp_dir (), "XXXXXX", NULL);
  mkdtemp (tmpdir);

//  g_assert (fd!=-1);
//  eh_print_on_error (error, "test_sequence_2");
//  g_assert (error==NULL);

  {
    //FILE* fp = fdopen (fd, "w");
    FILE* fp;
    gchar** line;

    name_used = g_build_filename (tmpdir, "sed_cube_test", NULL);

    fp = eh_fopen_error (name_used, "w" ,&error);
    eh_print_on_error (error, "test_sequence_2");

    for (line=test_seqfile; *line; line++ )
      fprintf (fp, "%s", *line);

    fclose (fp);
  }

  {
    GError* err = NULL;

    s = sed_get_floor_sequence_2 (name_used, y, n_y, &err);

    eh_print_on_error (err, "test_sequence_2");
  }

  g_assert (s!=NULL);
  g_assert_cmpint (s->len, ==, 2);

  eh_free (name_used);
  eh_free (tmpdir);
  //close (fd);
}

void
test_cube_to_cell (void)
{
  const int nx = g_test_rand_int_range (25, 50);
  const int ny = g_test_rand_int_range (25, 50);
  Sed_cube p = sed_cube_new (nx ,ny);
  Sed_cell dest;
  double cube_mass, cell_mass;

  sed_cube_set_x_res (p, 1.);
  sed_cube_set_y_res (p, 1.);
  sed_cube_set_z_res (p, 1.);

  {
    Sed_cell c = sed_cell_new_env ();
    gint i;

    sed_cell_set_equal_fraction (c);
    sed_cell_resize (c, 1.);

    for ( i=0 ; i<sed_cube_size (p) ; i++ )
      sed_column_add_cell (sed_cube_col(p,i), c);

    sed_cell_destroy (c);
  }

  dest = sed_cube_to_cell (p, NULL);

  cell_mass = sed_cell_mass (dest);
  cube_mass = sed_cube_mass (p);

  g_assert (eh_compare_dbl(cube_mass,cell_mass,1e-12));

  sed_cube_destroy( p );
}

void
test_cube_erode (void)
{
  const int nx = g_test_rand_int_range (25, 50);
  const int ny = g_test_rand_int_range (25, 50);
  const int len = nx*ny;
  Sed_cube p = sed_cube_new (nx ,ny);
  double *dz;

  g_assert (p);

  { /* Set erosion rates */
    int i;
    dz = eh_new (double, nx*ny);
    for (i=0; i<len; i++)
      dz[i] = g_test_rand_double_range (0, 10);
  }

  { /* Fill the cube with sediment */
    Sed_cell c = sed_cell_new_env ();
    gint i;

    g_assert (c);

    sed_cell_set_equal_fraction (c);

    for (i=0; i<len; i++)
    {
      sed_cell_resize (c, dz[i]*2.);
      sed_column_add_cell (sed_cube_col(p,i), c);
    }
  }

  { /* Erode the sediment */
    Sed_cube rtn;
    double mass_0 = sed_cube_mass (p);
    double mass_1;

    g_assert (mass_0>0);

    rtn = sed_cube_erode (p, dz);
    mass_1 = sed_cube_mass (p);

    g_assert (rtn==p);
    g_assert (eh_compare_dbl (mass_0, mass_1*2., 1e-12));
  }

  { /* NULL arguments */
    Sed_cube rtn;
    double mass_0 = sed_cube_mass (p);
    double mass_1;

    g_assert (mass_0>0);

    rtn = sed_cube_erode (p,NULL);
    mass_1 = sed_cube_mass (p);
    g_assert (eh_compare_dbl (mass_0, mass_1, 1e-12));
  }

  sed_cube_destroy (p);
}

void
test_cube_deposit (void)
{
  const int nx = g_test_rand_int_range (25, 50);
  const int ny = g_test_rand_int_range (25, 50);
  const int len = nx*ny;
  Sed_cube p = sed_cube_new (nx ,ny);
  Sed_cell *dz;

  g_assert (p);

  { /* Set deposit rates */
    int i;
    dz = eh_new (Sed_cell, nx*ny);
    for (i=0; i<len; i++)
    {
      dz[i] = sed_cell_new_env ();

      g_assert (dz[i]);
      sed_cell_set_equal_fraction (dz[i]);
      sed_cell_resize (dz[i], g_test_rand_double_range (0,10));
    }
  }

  { /* Fill the cube with sediment */
    Sed_cube rtn;
    double mass_0 = sed_cube_mass (p);
    double mass_1, mass_2;

    g_assert (eh_compare_dbl (mass_0, 0., 1e-12));

    rtn = sed_cube_deposit (p, dz);
    g_assert (rtn==p);

    mass_1 = sed_cube_mass (p);
    g_assert (mass_1>mass_0);

    sed_cube_deposit (p, dz);
    mass_2 = sed_cube_mass (p);
    g_assert (eh_compare_dbl (mass_2, 2.*mass_1, 1e-12));
  }

  { /* NULL arguments */
    Sed_cube rtn;
    double mass_0 = sed_cube_mass (p);
    double mass_1;

    g_assert (mass_0>0);

    rtn = sed_cube_deposit (p,NULL);
    mass_1 = sed_cube_mass (p);
    g_assert (eh_compare_dbl (mass_0, mass_1, 1e-12));
  }

  sed_cube_destroy (p);
}

void
test_cube_get_size (void)
{
  const int nx = g_test_rand_int_range (100, 200);
  const int ny = g_test_rand_int_range (100, 200);
  Sed_cube p = sed_cube_new (nx ,ny);

  g_assert (p);
  g_assert_cmpint (sed_cube_n_x (p), ==, nx);
  g_assert_cmpint (sed_cube_n_y (p), ==, ny);
  g_assert_cmpint (sed_cube_size (p), ==, nx*ny);

  p = sed_cube_destroy(p);

  g_assert (p==NULL);
}

void
test_cube_base_height (void)
{
  const int nx = g_test_rand_int_range (100,200);
  const int ny = g_test_rand_int_range (100,200);
  const int len = nx*ny;
  Sed_cube p = sed_cube_new (nx, ny);

  {
    int i, j;

    for (i=0; i<nx; i++)
      for (j=0; j<ny; j++)
        sed_cube_set_base_height (p, i, j, i*nx+j);

    for (i=0; i<nx; i++)
      for (j=0; j<ny; j++)
        g_assert (eh_compare_dbl (sed_cube_base_height(p, i, j),
                                  i*nx+j, 1e-12));
  }

  sed_cube_destroy (p);
}

void
test_cube_river_add (void)
{
  const int nx = g_test_rand_int_range (100,200);
  const int ny = g_test_rand_int_range (100,200);
  const int len = nx*ny;
  const int nland = ny/4;
  Sed_cube p = sed_cube_new (nx, ny);
  Sed_riv r = sed_river_new ("Trunk 1");

  g_assert (p);
  g_assert (r);

  { /* Set up the cube to have a shoreline */
    int i, j;

    for (i=0; i<nx; i++)
    {
      for (j=0; j<nland; j++)
        sed_cube_set_base_height (p, i, j, 1);
      for (j=nland; j<ny; j++)
        sed_cube_set_base_height (p, i, j, -1);
    }
  }

  { /* Find the river mouth of a river */
    Eh_ind_2 mouth;

    sed_river_set_hinge (r, nx/2, 0);
    sed_river_set_angle (r, G_PI*.5);
    sed_cube_find_river_mouth (p, r);

    mouth = sed_river_mouth (r);

    g_assert_cmpint (mouth.i, ==, nx/2);
    g_assert_cmpint (mouth.j, ==, nland);

    g_assert_cmpfloat (sed_cube_base_height(p, mouth.i, mouth.j), <, 0);
    g_assert_cmpfloat (sed_cube_base_height(p, mouth.i, mouth.j-1), >, 0);
  }

  { /* Add the river to a Sed_cube */
    Sed_riv* r_list = NULL;
    gpointer rtn = NULL;

    r_list = sed_cube_all_trunks (p);
    g_assert (r_list==NULL);

    rtn = sed_cube_add_trunk (p, r);
    g_assert (rtn);

    r_list = sed_cube_all_trunks (p);
    g_assert (r_list);
    g_assert_cmpint (g_strv_length ((gchar**)r_list), ==, 1);

    g_assert (r_list[0]!=r);
    g_assert (r_list[0]==rtn);
  }

  { /* Add another river to the Sed_cube */
    Sed_riv* r_list = NULL;
    Sed_riv r2 = sed_river_new ("Trunk 2");
    gpointer r2_id;

    sed_river_set_hinge (r2, nx/2, 0);
    sed_river_set_angle (r2, G_PI*.5);
    sed_cube_find_river_mouth (p, r2);

    r2_id = sed_cube_add_trunk (p, r2);

    r_list = sed_cube_all_trunks (p);
    g_assert (r_list);
    g_assert_cmpint (g_strv_length ((gchar**)r_list), ==, 2);

    g_assert (r_list[0]!=r2);
    g_assert (r_list[1]!=r);

    g_assert (sed_cube_nth_river (p, 0)!=r_list[0]);
    g_assert (sed_cube_nth_river (p, 1)!=r_list[1]);

    r2 = sed_river_destroy (r2);
  }

  { /* Move the river around in the Sed_cube */
    Eh_ind_2 mouth;
    Sed_riv r0 = sed_cube_nth_river (p, 0);

    sed_river_set_hinge (r0, nx/2, nland-1);
    sed_cube_find_river_mouth (p, r0);
    mouth = sed_river_mouth (r0);

    g_assert_cmpint (mouth.i, ==, nx/2);
    g_assert_cmpint (mouth.j, ==, nland);

    sed_river_set_hinge (r0, 0, nland-1);
    mouth = sed_river_mouth (r0);

    g_assert_cmpint (mouth.i, ==, nx/2);
    g_assert_cmpint (mouth.j, ==, nland);

    sed_cube_find_river_mouth (p, r0);
    mouth = sed_river_mouth (r0);

    g_assert_cmpint (mouth.i, ==, 0);
    g_assert_cmpint (mouth.j, ==, nland);

    sed_river_destroy (r0);
  }

  r = sed_river_destroy (r);
  sed_cube_destroy (p);
}

void
test_cube_river_north (void)
{
  const int nx = g_test_rand_int_range (100,200);
  const int ny = g_test_rand_int_range (100,200);
  const int len = nx*ny;
  const int nland = nx/4;
  Sed_cube p = sed_cube_new (nx, ny);
  Sed_riv r = sed_river_new ("North");

  g_assert (p);
  g_assert (r);

  { /* Set up the cube to have a shoreline */
    int i, j;

    for (i=0; i<nland; i++)
      for (j=0; j<ny; j++)
        sed_cube_set_base_height (p, i, j, 1);
    for (i=nland; i<nx; i++)
      for (j=0; j<ny; j++)
        sed_cube_set_base_height (p, i, j, -1);
  }

  { /* Find the river mouth of a river */
    Eh_ind_2 mouth;

    sed_river_set_hinge (r, 0, ny/2);
    sed_river_set_angle (r, 0);
    sed_cube_find_river_mouth (p, r);

    mouth = sed_river_mouth (r);

    g_assert_cmpint (mouth.i, ==, nland);
    g_assert_cmpint (mouth.j, ==, ny/2);

    g_assert_cmpfloat (sed_cube_base_height(p, mouth.i, mouth.j), <, 0);
    g_assert_cmpfloat (sed_cube_base_height(p, mouth.i-1, mouth.j), >, 0);
  }

  { /* Add the river to a Sed_cube */
    Sed_riv* r_list = NULL;

    r_list = sed_cube_all_trunks (p);
    g_assert (r_list==NULL);

    sed_cube_add_trunk (p, r);
    r_list = sed_cube_all_trunks (p);
    g_assert_cmpint (g_strv_length ((gchar**)r_list), ==, 1);

    g_assert (r_list[0]!=r);
  }

  r = sed_river_destroy (r);
  sed_cube_destroy (p);
}

void
test_river_new (void)
{
  gchar *name = g_strdup ("Test River");
  Sed_riv r = sed_river_new (name);

  g_assert (r);

  g_assert (name!=sed_river_name (r));
  g_assert (sed_river_name_is (r,name));
  g_assert (sed_river_name_cmp (r,name)==0);
  g_assert_cmpstr (sed_river_name (r), ==, name);

  g_assert (sed_river_mouth_is (r, 0, 0));

  g_assert (sed_river_hydro (r)==NULL);
  g_assert (!sed_river_has_children (r));
  g_assert (sed_river_right(r)==NULL);
  g_assert (sed_river_left(r)==NULL);

  g_assert (eh_compare_dbl (sed_river_angle (r), 0., 1e-12));
  g_assert (eh_compare_dbl (sed_river_min_angle (r), -G_PI, 1e-12));
  g_assert (eh_compare_dbl (sed_river_max_angle (r), G_PI, 1e-12));

  r = sed_river_destroy (r);

  g_assert (r==NULL);
}

void
test_river_dup (void)
{
  gchar *name = g_strdup ("Test River Dup");
  Sed_riv r = sed_river_new (name);
  Sed_riv r_copy = NULL;
  double min_angle = g_test_rand_double_range (-G_PI, G_PI);
  double max_angle = g_test_rand_double_range (min_angle, G_PI);
  double angle = g_test_rand_double_range (min_angle, max_angle);
  gint hinge_i = g_test_rand_int ();
  gint hinge_j = g_test_rand_int ();

  g_assert (r);

  sed_river_set_angle (r, angle);
  sed_river_set_angle_limit (r, min_angle, max_angle);
  sed_river_set_hinge (r, hinge_i, hinge_j);

  r_copy = sed_river_dup (r);

  g_assert (r_copy);
  g_assert (r_copy!=r);

  g_assert (eh_compare_dbl (sed_river_angle (r_copy),
                            sed_river_angle (r), 1e-12));
  g_assert (eh_compare_dbl (sed_river_min_angle (r_copy),
                            sed_river_min_angle (r), 1e-12));
  g_assert (eh_compare_dbl (sed_river_max_angle (r_copy),
                            sed_river_max_angle (r), 1e-12));

  sed_river_destroy (r_copy);
  sed_river_destroy (r);
}

void
test_river_angle (void)
{
  gchar *name = g_strdup ("Test River Angle");
  Sed_riv r = sed_river_new (name);

  g_assert (eh_compare_dbl (sed_river_angle (r), 0., 1e-12));
  g_assert (eh_compare_dbl (sed_river_min_angle (r), -G_PI, 1e-12));
  g_assert (eh_compare_dbl (sed_river_max_angle (r), G_PI, 1e-12));

  sed_river_set_angle (r, G_PI*.5);
  g_assert (eh_compare_dbl (sed_river_angle (r), G_PI*.5, 1e-12));

  sed_river_set_angle (r, G_PI*1.5);
  g_assert (eh_compare_dbl (sed_river_angle (r), -G_PI*.5, 1e-12));

  sed_river_set_angle (r, -G_PI);
  g_assert (eh_compare_dbl (sed_river_angle (r), -G_PI, 1e-12));

  sed_river_set_angle (r, G_PI);
  g_assert (eh_compare_dbl (sed_river_angle (r), G_PI-1e-12, 1e-12));

  sed_river_set_angle_limit (r, G_PI*.25, G_PI*.75);
  g_assert (eh_compare_dbl (sed_river_min_angle (r), G_PI*.25, 1e-12));
  g_assert (eh_compare_dbl (sed_river_max_angle (r), G_PI*.75, 1e-12));
  g_assert (eh_compare_dbl (sed_river_angle (r), G_PI*.75, 1e-12));

  sed_river_set_angle (r, 0.);
  g_assert (eh_compare_dbl (sed_river_angle (r), G_PI*.25, 1e-12));

  sed_river_increment_angle (r, G_PI*.25);
  g_assert (eh_compare_dbl (sed_river_angle (r), G_PI*.5, 1e-12));

  sed_river_destroy (r);
}

void
test_river_name (void)
{
  gchar *name = g_strdup_printf ("Test River %d", g_test_rand_int ());
  Sed_riv r = sed_river_new (name);
  gchar *name_ptr = NULL;

  g_assert (r);

  g_assert (name!=sed_river_name (r));
  g_assert (sed_river_name_is (r,name));
  g_assert (sed_river_name_cmp (r,name)==0);
  g_assert_cmpstr (sed_river_name (r), ==, name);

  name[0] = 't';
  g_assert (sed_river_name_is (r,name));

  name[0] = 'x';
  g_assert (!sed_river_name_is (r,name));

  name_ptr = sed_river_name_loc (r);
  g_assert (name_ptr!=name);
  g_assert (name_ptr!=sed_river_name (r));

  name_ptr[0] = 't';
  g_assert (sed_river_name_is (r, name_ptr));

  name_ptr[0] = 'X';
  g_assert (sed_river_name_is (r, name_ptr));

  sed_river_destroy (r);

}

void
test_river_hinge (void)
{
  Sed_riv r = sed_river_new ("Hinge Test");
  Sed_riv rtn = NULL;
  Eh_ind_2 hinge;
  gint i = g_test_rand_int ();
  gint j = g_test_rand_int ();

  g_assert (r);

  hinge = sed_river_hinge (r);

  g_assert_cmpint (hinge.i, ==, 0);
  g_assert_cmpint (hinge.j, ==, 0);

  rtn = sed_river_set_hinge (r, i, j);

  g_assert (rtn==r);
  hinge = sed_river_hinge (r);

  g_assert_cmpint (hinge.i, ==, i);
  g_assert_cmpint (hinge.j, ==, j);

  sed_river_destroy (r);
}

int
main (int argc, char* argv[])
{
  Sed_sediment s     = NULL;
  GError*      error = NULL;

  eh_init_glib ();

  s = sed_sediment_scan (SED_SEDIMENT_TEST_FILE, &error);

  if ( s )
    sed_sediment_set_env (s);
  else
    eh_error ("%s: Unable to read sediment file: %s",
              SED_SEDIMENT_TEST_FILE, error->message);

  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/libsed/sed_cube/new",&test_sed_cube_new);
  g_test_add_func ("/libsed/sed_cube/destroy",&test_sed_cube_destroy);
  g_test_add_func ("/libsed/sed_cube/sequence_2",&test_sequence_2);
  g_test_add_func ("/libsed/sed_cube/to_cell",&test_cube_to_cell);
  g_test_add_func ("/libsed/sed_cube/get_size",&test_cube_get_size);
  g_test_add_func ("/libsed/sed_cube/erode",&test_cube_erode);
  g_test_add_func ("/libsed/sed_cube/deposit",&test_cube_deposit);
  g_test_add_func ("/libsed/sed_cube/base_height",&test_cube_base_height);
  g_test_add_func ("/libsed/sed_cube/add_river",&test_cube_river_add);
  g_test_add_func ("/libsed/sed_cube/river_north",&test_cube_river_north);
  g_test_add_func ("/libsed/sed_river/new",&test_river_new);
  g_test_add_func ("/libsed/sed_river/dup",&test_river_dup);
  g_test_add_func ("/libsed/sed_river/angle",&test_river_angle);
  g_test_add_func ("/libsed/sed_river/name",&test_river_name);
  g_test_add_func ("/libsed/sed_river/hinge",&test_river_hinge);

  g_test_run ();
}

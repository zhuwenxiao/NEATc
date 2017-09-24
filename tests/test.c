#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <gtk/gtk.h>
#include <unistd.h>

#include <neat.h>

static double xor_inputs[4][2] = {{0.0, 0.0},
				  {0.0, 1.0},
				  {1.0, 0.0},
				  {1.0, 1.0}};
static double xor_outputs[4] = {0.0, 1.0, 1.0, 0.0};

static cairo_surface_t *surface = NULL;

static double calculate_fitness(neat_ffnet_t net)
{
	double fitness = 4.0;
	for(int i = 0; i < 4; i++){
		neat_ffnet_predict(net, xor_inputs[i]);
		double *outputs = neat_ffnet_get_outputs(net);
		neat_ffnet_reset(net);

		double diff = outputs[0] - xor_outputs[i];
		fitness -= diff * diff;
		free(outputs);
	}

	return fitness;
}

static void run()
{
	/* Initialize random numbers */
	srand(time(NULL));

	struct neat_config conf = {
		.fitness_criterion = NEAT_FITNESS_CRITERION_MEAN,
		.input_genome_topo = 2,
		.output_genome_topo = 1,

		.population_size = 15
	};

	neat_pop_t pop = neat_population_create(conf);

	neat_run(pop, calculate_fitness, 100);

	usleep(1000);
}

static void clear()
{
	cairo_t *cr = cairo_create(surface);

	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_paint(cr);

	cairo_destroy(cr);
}

static gboolean draw(GtkWidget *widget, cairo_t *cr, gpointer data)
{
	cairo_set_source_surface(cr, surface, 0, 0);
	cairo_paint(cr);

	return FALSE;
}

static gboolean configure(GtkWidget *widget, GdkEventConfigure *event,
			  gpointer data)
{
	if (surface){
		cairo_surface_destroy (surface);
	}

	GdkWindow *window = gtk_widget_get_window(widget);
	int width = gtk_widget_get_allocated_width(widget);
	int height = gtk_widget_get_allocated_height(widget);
	surface = gdk_window_create_similar_surface(window, CAIRO_CONTENT_COLOR,
						    width, height);

	clear();

	return TRUE;
}

static void close_window()
{
	if(surface){
		cairo_surface_destroy(surface);
	}
}

static void activate(GtkApplication *app, gpointer user_data)
{
	GtkWidget *window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(window), "NEATc");
	gtk_window_set_default_size(GTK_WINDOW(window), 300, 300);
	g_signal_connect(window, "destroy", G_CALLBACK(close_window), NULL);

	gtk_container_set_border_width(GTK_CONTAINER (window), 8);

	GtkWidget *frame = gtk_frame_new(NULL);
	gtk_container_add(GTK_CONTAINER(window), frame);

	GtkWidget *drawing_area = gtk_drawing_area_new();
	gtk_widget_set_size_request(drawing_area, 300, 300);
	gtk_container_add(GTK_CONTAINER(frame), drawing_area);
	g_signal_connect(drawing_area, "draw", G_CALLBACK(draw), NULL);
	g_signal_connect(drawing_area, "configure-event", G_CALLBACK(configure),
			 NULL);

	gtk_widget_show_all(window);

	run();
}

int main(int argc, char *argv[])
{
	run();
	return 0;

	GtkApplication *app = gtk_application_new("org.tversteeg.neatc",
						  G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

	int status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);

	return status;
}

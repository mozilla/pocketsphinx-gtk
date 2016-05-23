#include <gtk/gtk.h>
#include <pocketsphinx/pocketsphinx.h>
#include <iostream>
#include <sphinxbase/ad.h>
#include <sphinxbase/err.h>
#include <sys/select.h>

GtkWidget *progress_bar;
ps_decoder_t *ps;
int stop_sent = 0;

void ClickCallback(GtkWidget *widget, GdkEventButton *event, gpointer callback_data)
{
    // show which button was clicked
    std::cerr << "button pressed: "  << gtk_progress_bar_get_fraction(GTK_PROGRESS_BAR(progress_bar));
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), 0.01 + gtk_progress_bar_get_fraction(GTK_PROGRESS_BAR(progress_bar)));
}

/* Sleep for specified msec */
static void
sleep_msec(int32 ms)
{
#if (defined(_WIN32) && !defined(GNUWINCE)) || defined(_WIN32_WCE)
    Sleep(ms);
#else
    /* ------------------- Unix ------------------ */
    struct timeval tmo;

    tmo.tv_sec = 0;
    tmo.tv_usec = ms * 1000;

    select(0, NULL, NULL, NULL, &tmo);
#endif
}

/*
 * Main utterance processing loop:
 *     for (;;) {
 *        start utterance and wait for speech to process
 *        decoding till end-of-utterance silence will be detected
 *        print utterance result;
 *     }
 */
static void
recognize_from_microphone(cmd_ln_t *config)
{
    ad_rec_t *ad;
    int16 adbuf[2048];
    uint8 utt_started, in_speech;
    int32 k;
    char const *hyp;

    if ((ad = ad_open_dev(cmd_ln_str_r(config, "-adcdev"),
                          (int) cmd_ln_float32_r(config,
                                                 "-samprate"))) == NULL)
        E_FATAL("Failed to open audio device\n");
    if (ad_start_rec(ad) < 0)
        E_FATAL("Failed to start recording\n");

    if (ps_start_utt(ps) < 0)
        E_FATAL("Failed to start utterance\n");
    utt_started = FALSE;
    printf("READY....\n");

    for (;;) {
        if (stop_sent){
            break;
        }
        if ((k = ad_read(ad, adbuf, 2048)) < 0)
            E_FATAL("Failed to read audio\n");
        ps_process_raw(ps, adbuf, k, FALSE, FALSE);
        in_speech = ps_get_in_speech(ps);
        if (in_speech && !utt_started) {
            utt_started = TRUE;
            printf("Listening...\n");
        }
        if (!in_speech && utt_started) {
            /* speech -> silence transition, time to start new utterance  */
            ps_end_utt(ps);
            hyp = ps_get_hyp(ps, NULL );
            if (hyp != NULL)
                printf("%s\n", hyp);

            if (ps_start_utt(ps) < 0)
                E_FATAL("Failed to start utterance\n");
            utt_started = FALSE;
            printf("READY....\n");
        }
        sleep_msec(100);
    }
    ad_close(ad);
}

int pocketsphinxstart(){
    cmd_ln_t *config;
    config = cmd_ln_init(NULL, ps_args(), TRUE,
                         "-hmm", "/Users/anatal/projects/mozilla/vaani-iot/pocketsphinx/lib/ps/share/pocketsphinx/model/en-us/en-us",
                         "-keyphrase", "marieta",
                         "-dict", "/Users/anatal/projects/mozilla/vaani-iot/pocketsphinx/lib/ps/share/pocketsphinx/model/en-us/cmudict-en-us.dict",
                         "-kws_threshold", "1e-20",
                         NULL);
    if (config == NULL) {
        fprintf(stderr, "Failed to create config object, see log for details\n");
        return -1;
    }
    ps = ps_init(config);
    if (ps == NULL) {
        cmd_ln_free_r(config);
        return 1;
    }

    E_INFO("COMPILED ON: %s, AT: %s\n\n", __DATE__, __TIME__);

    recognize_from_microphone(config);

    ps_free(ps);
    cmd_ln_free_r(config);

}

int render_gtk(int argc, char *argv[]){
    GtkWidget *window;
    GtkWidget *label;
    GtkWidget *button_start;
    GtkWidget* hbox;

    gtk_init(&argc, &argv);

    /* Create the main, top level window */
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    /* Give it the title */
    gtk_window_set_title(GTK_WINDOW(window), "Vaani");

    /* Center the window */
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    /* Set the window's default size */
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 100);

    /*
    ** Map the destroy signal of the window to gtk_main_quit;
    ** When the window is about to be destroyed, we get a notification and
    ** stop the main GTK+ loop by returning 0
    */
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    /*
    ** Assign the variable "label" to a new GTK label,
    ** with the text "Hello, world!"
    */
    label = gtk_label_new("Hello, world!");

    // Add the button onto the main window
    button_start = gtk_button_new_with_label ("Touch to start listening");

    hbox = gtk_hbox_new(true, 0);

    progress_bar = gtk_progress_bar_new ();

    gtk_container_add(GTK_CONTAINER(hbox), button_start);
    /* Plot the label onto the main window */
    gtk_container_add(GTK_CONTAINER(hbox), label);
    gtk_container_add(GTK_CONTAINER(hbox), progress_bar);

    // add the hbox to the window
    gtk_container_add(GTK_CONTAINER(window), hbox);
    g_signal_connect(G_OBJECT(button_start), "button_press_event", G_CALLBACK(ClickCallback), NULL);

    /* Make sure that everything, window and label, are visible */
    gtk_widget_show_all(window);

    pocketsphinxstart();

    /*
    ** Start the main loop, and do nothing (block) until
    ** the application is closed
    */
    gtk_main();
}

int main (int argc, char *argv[])
{
    render_gtk(argc, argv);
    return 0;
}
//
// Created by Andre Natal on 5/23/16.
//

#include <pocketsphinx/pocketsphinx.h>
#include <sphinxbase/ad.h>
#include <sphinxbase/err.h>
#include <sys/select.h>
#include <pthread.h>


ps_decoder_t *ps;
int stop_sent = 0;
cmd_ln_t *config;

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
void *
recognize_from_microphone(void *args)
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

    return NULL;
}

int pocketsphinxstart(){
    config = cmd_ln_init(NULL, ps_args(), TRUE,
                         "-hmm", "/Users/anatal/projects/mozilla/vaani-iot/pocketsphinx/lib/ps/share/pocketsphinx/model/en-us/en-us",
                         "-keyphrase", "ok vaani",
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

   // recognize_from_microphone(config);

}

void destroy_ps(){
    ps_free(ps);
    cmd_ln_free_r(config);
}
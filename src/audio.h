#include <jack/ringbuffer.h>

/* These ringbuffers will contain 8000Hz 16-bit data (actually ideally it
   is/would be gsm_signal which is 13-bit) */
jack_ringbuffer_t *input_rb;	// i.e. mouthpiece
jack_ringbuffer_t *output_rb;	// i.e. earpiece

#include "protocol.h"
int big_endian_decode(uint8_t const *buffer, int size){
    int value = 0;
    for (int i = 0; i < size; i++) {
        value |= buffer[i] << (8 * (size - i - 1));
    }
    return value;
}

void big_endian_encode(int value, uint8_t *buffer, int size) {
    for (int i = 0; i < sizeof(int); i++) {
        buffer[i] = (value >> (8 * (sizeof(int) - i - 1))) & 0xFF;
    }
}

float decode_float(uint8_t *buffer) {
	int value = big_endian_decode(buffer, TYPST_INT_SIZE);
	if (value == 0) {
		return 0.0f;
	}
	union FloatBuffer {
		float f;
		int i;
	} float_buffer;
	float_buffer.i = value;
	return float_buffer.f;
}

void encode_float(float value, uint8_t *buffer) {
	if (value == 0.0f) {
		big_endian_encode(0, buffer, TYPST_INT_SIZE);
	} else {
		union FloatBuffer {
			float f;
			int i;
		} float_buffer;
		float_buffer.f = value;
		big_endian_encode(float_buffer.i, buffer, TYPST_INT_SIZE);
	}
}

size_t list_size(void *list, size_t size, size_function sf, size_t element_size) {
    size_t result = 0;
    for (int i = 0; i < size; i++) {
        result += sf(list + i * element_size);
    }
    return result;
}

size_t optional_size(void *opt, size_function sf) {
    return 1 + (opt ? sf(opt) : 0);
}

size_t int_size(const void* elem) {
    return TYPST_INT_SIZE;
}
size_t float_size(const void *elem) {
    return TYPST_INT_SIZE;
}
size_t bool_size(const void *elem) {
    return TYPST_INT_SIZE;
}
size_t char_size(const void *elem) {
    return 1;
}
size_t string_size(const void *elem) {
    if (!elem || !((char *)elem)[0]) {
        return 1;
    }
    return strlen((char *)elem) + 1;
}
size_t string_list_size(char **list, size_t size) {
	size_t result = 0;
	for (size_t i = 0; i < size; i++) {
		result += string_size(list[i]);
	}
	return result;
}

void free_Params(Params *s) {
    free(s->initial_guess);
}
int decode_Params(uint8_t *__input_buffer, size_t buffer_len, Params *out, size_t *buffer_offset) {
    size_t __buffer_offset = 0;
    int err;
    (void)err;
    NEXT_INT(out->initial_guess_len)
    if (out->initial_guess_len == 0) {
        out->initial_guess = NULL;
    } else {
        out->initial_guess = malloc(out->initial_guess_len * sizeof(float));
        if (!out->initial_guess){
            return 1;
        }
        for (size_t i = 0; i < out->initial_guess_len; i++) {
    NEXT_FLOAT(out->initial_guess[i])
        }
    }
    NEXT_FLOAT(out->tolerance)
    NEXT_INT(out->max_iterations)
    *buffer_offset += __buffer_offset;
    return 0;
}
void free_Result(Result *s) {
    free(s->minimum_point);
}
size_t Result_size(const void *s){
	return TYPST_INT_SIZE + list_size(((Result*)s)->minimum_point, ((Result*)s)->minimum_point_len, int_size, sizeof(*((Result*)s)->minimum_point)) + TYPST_INT_SIZE + TYPST_INT_SIZE + 1;
}
int encode_Result(const Result *s, uint8_t *__input_buffer, size_t *buffer_len, size_t *buffer_offset) {
    size_t __buffer_offset = 0;    size_t s_size = Result_size(s);
    if (s_size > *buffer_len) {
        return 2;
    }
    int err;
	(void)err;
    INT_PACK(s->minimum_point_len)
    for (size_t i = 0; i < s->minimum_point_len; i++) {
    FLOAT_PACK(s->minimum_point[i])
    }
    FLOAT_PACK(s->minimum_value)
    INT_PACK(s->iterations)
    CHAR_PACK(s->success)

    *buffer_offset += __buffer_offset;
    return 0;
}
void free_Request(Request *s) {
    free_Params(&s->params);
}
int decode_Request(size_t buffer_len, Request *out) {
    INIT_BUFFER_UNPACK(buffer_len)
    int err;
    (void)err;
    if ((err = decode_Params(__input_buffer + __buffer_offset, buffer_len - __buffer_offset, &out->params, &__buffer_offset))){return err;}
    FREE_BUFFER()
    return 0;
}
void free_Response(Response *s) {
    free_Result(&s->result);
}
size_t Response_size(const void *s){
	return Result_size((void*)&((Response*)s)->result);
}
int encode_Response(const Response *s) {
    size_t buffer_len = Response_size(s);
    INIT_BUFFER_PACK(buffer_len)
    int err;
	(void)err;
        if ((err = encode_Result(&s->result, __input_buffer + __buffer_offset, &buffer_len, &__buffer_offset))) {
            return err;
        }

    wasm_minimal_protocol_send_result_to_host(__input_buffer, buffer_len);
    return 0;
}

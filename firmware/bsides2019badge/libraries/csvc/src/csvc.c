#include "csvc.h"

CSVC_BOOL csvc_dimensions(size_t size, CsvcReadFn read_fn, void *read_ctx, size_t *rows, size_t *columns) {
    size_t comma_count = 0;
    size_t char_offset = 0;
    size_t row_count = 0;
    size_t column_count = 0;
    char char_value = '\0';
    if (!read_fn) {
        return CSVC_FALSE;
    }
    for (char_offset = 0; char_offset < size; char_offset++) {
        if (0 == row_count) {
            // we are on the first row
            row_count = 1;
        }
        if (1 != read_fn(read_ctx, char_offset, &char_value, sizeof(char))) {
            return CSVC_FALSE;
        }
        // if its a comma, increase the count
        if (char_value == ',') {
            comma_count++;
        }
        // check for maximum commas on newline
        if (char_value == '\n') {
            if (comma_count > column_count) {
                column_count = comma_count;
            }
            // reset counter for next line
            comma_count = 0;
            // count rows/newlines
            row_count++;
        }
        // exit on nul-terminator
        if (char_value == '\0') {
            break;
        }
    }
    // check for maximum on the last line
    if (comma_count > column_count) {
        column_count = comma_count;
    }
    // always 1 more column then there are commas
    column_count++;
    if (rows) {
        *rows = row_count;
    }
    if (columns) {
        *columns = column_count;
    }
    return CSVC_TRUE;
}

CSVC_BOOL csvc_for_each_cell(size_t size, CsvcReadFn read_fn, void *read_ctx, CsvcCellFn cell_fn, void *cell_ctx, char *buffer, size_t buffer_size) {
    size_t row = 0;
    size_t column = 0;
    size_t buffer_used = 0;
    size_t char_offset = 0;
    char char_value = '\0';
    if (!read_fn) {
        return CSVC_FALSE;
    }
    if (!cell_fn) {
        return CSVC_TRUE;
    }
    if ((!buffer) || (0 == buffer_size)) {
        return CSVC_FALSE;
    }
    buffer[0] = '\0';
    buffer_used = 0;
    for (char_offset = 0; char_offset < size; char_offset++) {
        if (1 != read_fn(read_ctx, char_offset, &char_value, sizeof(char))) {
            return CSVC_FALSE;
        }
        if ((char_value == ',') || (char_value == '\n') || (char_value == '\0')) {
            // make the text nul-terminated
            buffer[buffer_used] = '\0';
            // callback
            if (CSVC_TRUE != cell_fn(cell_ctx, row, column, buffer)) {
                break;
            }
            // reset buffer
            buffer[0] = '\0';
            buffer_used = 0;
            // next value
            if (char_value == ',') {
                // move to next column
                column++;
            }
            if (char_value == '\n') {
                // move to next line
                column = 0;
                row++;
            }
            if (char_value == '\0') {
                // consider it EOF
                return CSVC_TRUE;
            }
        } else {
            // just another character
            // always 1 less then buffer size (as we need a nul-terminator)
            if (buffer_used < (buffer_size - 1)) {
                buffer[buffer_used] = char_value;
                buffer_used++;
            }
            // always nul-terminated
            if (buffer_used >= buffer_size) {
                buffer_used = buffer_size;
                buffer[buffer_used - 1] = '\0';
            } else {
                buffer[buffer_used] = '\0';
            }
        }
    }
    return CSVC_TRUE;
}

CSVC_BOOL csvc_read_cell(size_t size, CsvcReadFn read_fn, void *read_ctx, size_t row, size_t column, char *buffer, size_t buffer_size)
{
    size_t current_row = 0;
    size_t current_column = 0;
    size_t buffer_used = 0;
    size_t char_offset = 0;
    char char_value = '\0';
    if (!read_fn) {
        return CSVC_FALSE;
    }
    if ((!buffer) || (0 == buffer_size)) {
        return CSVC_FALSE;
    }
    buffer[0] = '\0';
    buffer_used = 0;
    for (char_offset = 0; char_offset < size; char_offset++) {
        if (1 != read_fn(read_ctx, char_offset, &char_value, sizeof(char))) {
            return CSVC_FALSE;
        }
        if ((current_column == column) && (current_row == row)) {
            if ((char_value == ',') || (char_value == '\n') || (char_value == '\0')) {
                // done
                return CSVC_TRUE;
            }
            // store char
            if (buffer_used < buffer_size) {
                buffer[buffer_used] = char_value;
                buffer_used++;
            }
            // always nul-terminated
            if (buffer_used >= buffer_size) {
                buffer_used = buffer_size;
                buffer[buffer_used - 1] = '\0';
            } else {
                buffer[buffer_used] = '\0';
            }
            // stop if we can't read anymore
            if (buffer_used == buffer_size) {
                // done
                return CSVC_TRUE;
            }
        } else {
            if (char_value == ',') {
                current_column++;
            }
            if (char_value == '\n') {
                current_row++;
                current_column = 0;
            }
            if (char_value == '\0') {
                return CSVC_TRUE;
            }
        }
    }
    return CSVC_TRUE;
}

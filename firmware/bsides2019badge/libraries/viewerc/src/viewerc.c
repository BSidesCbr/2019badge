#include "viewerc.h"

VWRC_BOOL vwrc_init(void *data, uint32_t size)
{
    if (size < sizeof(VWRC_VIEWER_DATA))
    {
        return VWRC_FALSE;
    }
    memset(data, 0, size);
    ((VWRC_VIEWER_DATA*)data)->max_chars_per_line = (size - sizeof(VWRC_VIEWER_DATA));
    return VWRC_TRUE;
}

static int32_t vwrc_read_row(VWRC_VIEWER_DATA *viewer, uint32_t offset, char *buffer, uint32_t buffer_size) {
    char c = ' ';
    uint32_t total_recd = 0;
    uint32_t recd = 0;
    uint32_t row_width = 0;
    uint32_t unused = 0;

    // make sure there is space for at-least 1 byte
    // and we need this byte for the nul-terminator
    if (0 == buffer_size) {
        return -1;
    }

    // clear the buffer (cause I like a clean-slate)
    memset(buffer, 0, buffer_size);

    // read up to buffer size (-1 for the nul-terminator)
    while (total_recd < (buffer_size - 1)) {
        // read a character at at time (cause I'm lazy / un-optimized)
        recd = viewer->read_data(viewer->read_data_ctx, offset + total_recd, &c, sizeof(char));

        // exit (with nothing) on error
        if (recd < 0) {
            return -1;
        }

        // exit (with nothing), we are only reading a single character
        if (recd != sizeof(char)) {
            return -1;
        }

        // ignore null-terminator
        // we use the supplied total chars as the authority
        if ('\0' == c) {
            c = ' ';
        }

        // lazy again, tabs are treated as spaces
        if ('\t' == c) {
            c = ' ';
        }

        // yay! we read some data
        buffer[total_recd] = c;
        total_recd += (uint32_t)recd;

        // have we reached a new-line separator?
        if ('\n' == c) {
            break;
        }

        // has all data has been read?
        if ((offset + total_recd) >= viewer->total_chars) {
            break;
        }

        // have we read more text then fits on the row?
        viewer->calc_string_view(viewer->calc_string_view_ctx, buffer, &row_width, &unused);
        if (row_width > viewer->view_width) {
            // go back 1 character so we fit
            if (total_recd > 0) {
                total_recd -= 1;
            }
            break;
        }

        // keep reading until we read the limits of our buffer
    }

    // make sure the buffer is nul-terminated
    buffer[total_recd] = '\0';

    // empty line == we are done
    if (0 == total_recd) {
        return (int32_t)total_recd;
    }

    // if we reached the new-line character
    if ('\n' == buffer[total_recd-1]) {
        // change the new-line character to nul-terminator
        // make that the end of the row
        buffer[total_recd-1] = '\0';

        // don't adjust the count, we have consumed this character!
        // we used it to goto a new line
        return (int32_t)total_recd;
    }

    // if the last character is not a space, we need to word-wrap
    if ((' ' != buffer[total_recd - 1])) {
        // calculate the size of the word
        recd = 0;
        while (((total_recd - recd) > 0) && (' ' != buffer[total_recd - 1 - recd])) {
            recd++;
        }
        // if the size of the word is greater than a single line, ignore word-wrap
        if (recd >= total_recd) {
            // ignore word-wrap, it fills a whole line
        } else {
            // just don't do it for the last row
            if ((offset + total_recd) >= viewer->total_chars) {
                // last row, don't wrap
            } else {
                // word wrap
                total_recd -= recd;
            }
        }
    }

    // nul-terminate at this new point
    buffer[total_recd] = '\0';

    // return the total characters read for this line
    return (int32_t)total_recd;
}

static void vwrc_recalculate(VWRC_VIEWER_DATA *viewer) {
    uint32_t index = 0;
    int32_t recd = 0;
    uint32_t unused = 0;
    uint32_t line_height = 0;

    // use 'j' as it is usually the 'longer' one in vertical height
    char string_for_height[2] = {'j', '\0'};

    // reset
    viewer->row = 0;
    viewer->row_count = 0;
    viewer->rows_per_view = 0;

    // must have 'all the things' before I can calcualte
    if (0 == viewer->total_chars) {
        return;
    }
    if (0 == viewer->view_width) {
        return;
    }
    if (0 == viewer->view_height) {
        return;
    }
    if (!(viewer->read_data)) {
        return;
    }
    if (!(viewer->calc_string_view)) {
        return;
    }
    if (!(viewer->read_data)) {
        return;
    }

    // determine the height of a single text line
    viewer->calc_string_view(viewer->calc_string_view_ctx, string_for_height, &unused, &line_height);
    viewer->rows_per_view = viewer->view_height / line_height;

    // if the view is smaller then text, not much we can do
    if (0 == viewer->rows_per_view) {
        return;
    }

    // count the number of view positions
    // a view position is kind of like a place on a scroll bar
    // when you move a scroll bar up and down, each position
    // shows a different view of the text
    while (index < viewer->total_chars) {
        // we have been told the total number of text characters in this view
        // now we need to read rows (so we can count them)

        // NOTE: we can +1 to max_chars_per_line because the struct has 'char buffer[1];'
        //       this creates an extra byte before the actual buffer.
        //       we will use this to store the null character for the buffer
        recd = vwrc_read_row(viewer, index, viewer->buffer, viewer->max_chars_per_line + 1);
        if (recd < 0) {
            // a read failed

            // reset the row count
            viewer->row_count = 0;

            // exit
            return;
        }

        // we have read (received) this number of characters for this row.
        // move our index in the text along
        index += (uint32_t)recd;

        // this row is part of a view in the viewer
        viewer->row_count += 1;
    }

    // start display on first row
    viewer->row = 0;
}

VWRC_BOOL vwrc_set_view(void *data, uint32_t width, uint32_t height)
{
    VWRC_VIEWER_DATA *viewer = (VWRC_VIEWER_DATA *)data;
    if (NULL == viewer) {
        return VWRC_FALSE;
    }
    viewer->view_width = width;
    viewer->view_height = height;
    vwrc_recalculate(viewer);
    return VWRC_TRUE;
}

VWRC_BOOL vwrc_set_calc_string_view(void *data, VwrcCalcStringViewFn func, void *ctx)
{
    VWRC_VIEWER_DATA *viewer = (VWRC_VIEWER_DATA *)data;
    if (NULL == viewer) {
        return VWRC_FALSE;
    }
    viewer->calc_string_view = func;
    viewer->calc_string_view_ctx = ctx;
    vwrc_recalculate(viewer);
    return VWRC_TRUE;
}

VWRC_BOOL vwrc_set_draw_string(void *data, VwrcDrawStringFn func, void *ctx) {
    VWRC_VIEWER_DATA *viewer = (VWRC_VIEWER_DATA *)data;
    if (NULL == viewer) {
        return VWRC_FALSE;
    }
    viewer->draw_string = func;
    viewer->draw_string_ctx = ctx;
    vwrc_recalculate(viewer);
    return VWRC_TRUE;
}

VWRC_BOOL vwrc_set_text(void *data, uint32_t total_chars, VwrcReadFn func, void *ctx) {
    VWRC_VIEWER_DATA *viewer = (VWRC_VIEWER_DATA *)data;
    if (NULL == viewer) {
        return VWRC_FALSE;
    }
    viewer->total_chars = total_chars;
    viewer->read_data = func;
    viewer->read_data_ctx = ctx;
    vwrc_recalculate(viewer);
    return VWRC_TRUE;
}

VWRC_BOOL vwrc_get_row(void *data, uint32_t *row) {
    VWRC_VIEWER_DATA *viewer = (VWRC_VIEWER_DATA *)data;
    if (NULL == viewer) {
        return VWRC_FALSE;
    }
    if (row) {
        *row = viewer->row;
    }
    return VWRC_TRUE;
}

VWRC_BOOL vwrc_get_row_count(void *data, uint32_t *rows) {
    VWRC_VIEWER_DATA *viewer = (VWRC_VIEWER_DATA *)data;
    if (NULL == viewer) {
        return VWRC_FALSE;
    }
    if (rows) {
        *rows = viewer->row_count;
    }
    return VWRC_TRUE;
}

VWRC_BOOL vwrc_get_rows_per_view(void *data, uint32_t *rows) {
    VWRC_VIEWER_DATA *viewer = (VWRC_VIEWER_DATA *)data;
    if (NULL == viewer) {
        return VWRC_FALSE;
    }
    if (rows) {
        *rows = viewer->rows_per_view;
    }
    return VWRC_TRUE;
}

VWRC_BOOL vwrc_scroll_up(void *data) {
    VWRC_VIEWER_DATA *viewer = (VWRC_VIEWER_DATA *)data;
    if (NULL == viewer) {
        return VWRC_FALSE;
    }
    if (viewer->row > 0) {
        viewer->row -= 1;
    }
    return VWRC_TRUE;
}

VWRC_BOOL vwrc_scroll_to_row(void *data, uint32_t row) {
    VWRC_VIEWER_DATA *viewer = (VWRC_VIEWER_DATA *)data;
    if (NULL == viewer) {
        return VWRC_FALSE;
    }
    if (viewer->rows_per_view > viewer->row_count) {
        // there is more display area than text
        // so just go to the first row
        viewer->row = 0;
    } else if (row >= (viewer->row_count - viewer->rows_per_view)) {
        // the row is within the last possible view / display
        // so cap it to the first row in the last view
        viewer->row = viewer->row_count - viewer->rows_per_view;
    } else {
        // valid row
        viewer->row = row;
    }
    return VWRC_TRUE;
}

VWRC_BOOL vwrc_scroll_down(void *data) {
    VWRC_VIEWER_DATA *viewer = (VWRC_VIEWER_DATA *)data;
    if (NULL == viewer) {
        return VWRC_FALSE;
    }
    return vwrc_scroll_to_row(data, viewer->row + 1);
}

VWRC_BOOL vwrc_draw_view(void *data) {
    uint32_t current_row = 0;
    uint32_t index = 0;
    int32_t recd = 0;
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t unused = 0;
    uint32_t line_height = 0;

    // use 'j' as it is usually the 'longer' one in vertical height
    char string_for_height[2] = {'j', '\0'};

    VWRC_VIEWER_DATA *viewer = (VWRC_VIEWER_DATA *)data;
    if (NULL == viewer) {
        return VWRC_FALSE;
    }

    // need the apis
    if (!(viewer->draw_string)) {
        return VWRC_FALSE;
    }
    if (!(viewer->calc_string_view)) {
        return VWRC_FALSE;
    }
    if (!(viewer->read_data)) {
        return VWRC_FALSE;
    }

    // need the line height
    viewer->calc_string_view(viewer->calc_string_view_ctx, string_for_height, &unused, &line_height);

    // find and draw the rows
    while (index < viewer->total_chars) {
        // NOTE: we can +1 to max_chars_per_line because the struct has 'char buffer[1];'
        //       this creates an extra byte before the actual buffer.
        //       we will use this to store the null character for the buffer
        recd = vwrc_read_row(viewer, index, viewer->buffer, viewer->max_chars_per_line + 1);
        if (recd < 0) {
            // a read failed
            // exit
            return VWRC_FALSE;
        }

        // we have read (received) this number of characters for this row.
        // move our index in the text along
        index += (uint32_t)recd;

        // display the rows that are in the current view
        if ((current_row >= viewer->row) && (current_row < (viewer->row + viewer->rows_per_view))) {
            viewer->draw_string(viewer->draw_string_ctx, x, y, viewer->buffer);
            y += line_height;
        }

        // next row
        current_row++;
    }

    // done
    return VWRC_TRUE;
}

VWRC_BOOL vwrc_fini(void *data)
{
    uint32_t size = sizeof(VWRC_VIEWER_DATA);
    if (NULL == data) {
        return VWRC_FALSE;
    }
    memset(data, 0, size);
    return VWRC_TRUE;
}

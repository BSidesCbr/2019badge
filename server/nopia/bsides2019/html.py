import os

class HtmlTable(object):

    def __init__(self, table=None, headings=None):
        self.headings = headings
        self.table = table

    def html(self):
        text = ''
        text += '<table>'
        if self.headings:
            text += '<thead>'
            text += '<tr>'
            for heading in self.headings:
                text += '<th>{}</th>'.format(heading)
            text += '</tr>'
            text += '</thead>'
        text += '<tbody>'
        for row in self.table:
            text += '<tr>'
            for cell in row:
                text += '<td>{}</td>'.format(cell)
            text += '</tr>'
        text += '</tbody>'
        text += '</table>'
        return text

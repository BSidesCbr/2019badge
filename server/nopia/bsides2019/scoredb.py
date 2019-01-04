import os
import datetime
import MySQLdb

class ScoreDb(object):

    def __init__(self, version):
        self.db = None
        self.version = version

    def __enter__(self):   
        self.db = MySQLdb.connect(
            host=os.environ['RDS_HOSTNAME'],
            port=int(os.environ['RDS_PORT']),
            user=os.environ['RDS_USERNAME'],
            passwd=os.environ['RDS_PASSWORD'],
            db=os.environ['RDS_DB_NAME'])
        try:
            self.init_db()
        except:
            self.db.close()
            raise
        return self

    def __exit__(self ,type, value, traceback):
        self.db.close()
        self.db = None

    def sql_write(self, cmd):
        cursor = self.db.cursor()
        try:
            try:
                cursor.execute(cmd)
                self.db.commit()
            except:
                self.db.rollback()
                raise
        except Exception as error:
            error = str(error).lower()
            if error.find('already exists') != -1:
                return
            raise

    def sql_read(self, cmd, count=None):
        cursor = self.db.cursor()
        try:
            cursor.execute(cmd)
            if count == 1:
                results = [cursor.fetchone()]
            else:
                results = cursor.fetchall()
            list(results)
            if count:
                results = results[:count]
        except:
            raise
        new_results = list()
        for row in results:
            new_results.append(list(row))
        results = new_results
        return results

    def sql_table_name(self, name):
        table_name = '{}_V{}'.format(name, self.version).upper()
        return table_name

    def sql_version(self):
        return self.sql_read("SELECT VERSION()", 1)[0][0]

    def sql_tables(self):
        results = self.sql_read('show tables;')
        tables = list()
        for row in results:
            for col in row:
                tables.append(col)
        return tables

    def score_table_name(self, game):
        return self.sql_table_name(game + '_score')

    def name_table_name(self):
        return self.sql_table_name('name')

    def create_score_table(self, game):
        sql = """
            CREATE TABLE {}
            (
                IMEI        CHAR(20) NOT NULL,
                SCORE       INT,
                TIMESTAMP   DATETIME
            )""".format(self.score_table_name(game))
        self.sql_write(sql)

    def create_name_table(self):
        sql = """
            CREATE TABLE {}
            (
                IMEI        CHAR(20) NOT NULL PRIMARY KEY,
                NAME        CHAR(20) NOT NULL
            )""".format(self.name_table_name())
        self.sql_write(sql)

    def add_name(self, imei, name):
        timestamp = datetime.datetime.now()
        timestamp = timestamp.strftime('%Y-%m-%d %H:%M:%S')
        sql = """
            INSERT INTO {} (
                IMEI,
                NAME
            )
            VALUES('{}', '{}')
            ON DUPLICATE KEY UPDATE NAME = '{}'
        """.format(
            self.name_table_name(),
            imei,
            name,
            name
        )
        self.sql_write(sql)

    def add_score(self, imei, game, score):
        timestamp = datetime.datetime.now()
        timestamp = timestamp.strftime('%Y-%m-%d %H:%M:%S')
        sql = """
            INSERT INTO {} (
                IMEI,
                SCORE,
                TIMESTAMP
            )
            VALUES('{}', {}, '{}' )""".format(
                self.score_table_name(game),
                imei,
                score,
                timestamp
            )
        self.sql_write(sql)

    def score_board(self, game, count=None):
        tscore = self.score_table_name(game)
        tname = self.name_table_name()
        sql = """
            SELECT
                IF({}.NAME IS NULL, {}.IMEI, {}.NAME) as IMEI,
                {}.SCORE as SCORE,
                {}.TIMESTAMP as TIMESTAMP
            FROM {}
            LEFT JOIN {} ON {}.IMEI = {}.IMEI
            ORDER BY SCORE DESC, TIMESTAMP ASC
        """.format(
            tname,  # is name null?
            tscore, # no name, use imei
            tname,  # use name as imei
            tscore, # score
            tscore, # timestamp
            tscore,
            tname,
            tscore,
            tname,
        )
        results = self.sql_read(sql, count=count)
        new_rows = list()
        for row in results:
            new_rows.append((row[0], str(row[1]) + ' pts'))
        results = new_rows
        return results

    def name_table(self, count=None):
        sql = """
            SELECT IMEI, NAME FROM {}
        """.format(self.name_table_name())
        return self.sql_read(sql, count=count)

    def init_db(self):
        self.create_name_table()
        self.create_score_table('test')
        #self.create_score_table('snake')
        #self.create_score_table('tetris')

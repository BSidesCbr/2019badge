import os
import uuid
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
                results = cursor.fetchone()
                if results:
                    results = [results]
                else:
                    results = list()
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

    def sql_drop_table(self, name):
        sql = "DROP TABLE IF EXISTS {}".format(name)
        self.sql_write(sql)

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
        # add in defaults scores
        if len(self.score_board(game, count=1)) == 0:
            for i in range(0, 20):
                num1 = int(i % 10)
                num2 = int(i / 10)
                digit1 = chr(ord('0') + num1)
                digit2 = chr(ord('0') + num2)
                imei = '3534'
                for _ in range(0, 6):
                    imei += str(int(os.urandom(1)[0] % 100)).rjust(2, '0')
                name = chr(ord('A') + i) * 3
                assert len(imei) == 16
                assert len(name) == 3
                score = (20 - i) * 100
                assert score % 100 == 0
                self.add_score(imei, game, score)
                self.add_name(imei, name)

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
        # try to avoid duplicates of the same imei score
        # timestamp will help to resolve these later if it does happen
        sql = """
            SELECT IMEI, SCORE FROM {} WHERE IMEI = '{}' AND SCORE = {}
        """.format(
            self.score_table_name(game),
            imei,
            score
        )
        results = self.sql_read(sql)
        if len(results) > 0:
            # don't add another imei/score of the same values
            # user already has this score recorded
            return
        # add score
        timestamp = datetime.datetime.now()
        timestamp = timestamp.strftime('%Y-%m-%d %H:%M:%S')
        sql = """
            INSERT INTO {} (
                IMEI,
                SCORE,
                TIMESTAMP
            )
            VALUES('{}', {}, '{}')
            """.format(
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
            SELECT * FROM 
            (
                SELECT
                    {}.IMEI as IMEI,
                    {}.NAME as NAME,
                    {}.SCORE as SCORE,
                    {}.TIMESTAMP as TIMESTAMP
                FROM {}
                LEFT JOIN {} ON {}.IMEI = {}.IMEI
                ORDER BY SCORE DESC, TIMESTAMP ASC
            ) AS A
            RIGHT JOIN
            (
                SELECT IMEI, MAX(SCORE) AS MAX_SCORE FROM {} GROUP BY IMEI
            ) AS B
            ON A.IMEI = B.IMEI AND A.SCORE = B.MAX_SCORE
            ORDER BY SCORE DESC, TIMESTAMP ASC
        """.format(
            tscore, # imei
            tname,  # name
            tscore, # score
            tscore, # timestamp
            tscore,
            tname,
            tscore,
            tname,
            tscore, # for max score
        )
        return self.sql_read(sql, count=count)

    def name_table(self, count=None):
        sql = """
            SELECT IMEI, NAME FROM {}
        """.format(self.name_table_name())
        return self.sql_read(sql, count=count)

    def reset_db(self):
        self.sql_drop_table(self.name_table_name())
        self.sql_drop_table(self.score_table_name('snake'))
        self.sql_drop_table(self.score_table_name('tetris'))

    def init_db(self):
        self.create_name_table()
        self.create_score_table('snake')
        self.create_score_table('tetris')

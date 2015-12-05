#include <QString>
#include <QtTest>

class CSVWriterTest : public QObject
{
    Q_OBJECT

public:
    CSVWriterTest();

private Q_SLOTS:
    void testWrite1();
};

CSVWriterTest::CSVWriterTest()
{
}

void CSVWriterTest::testWrite1()
{
    QVERIFY2(true, "Failure");
}

QTEST_APPLESS_MAIN(CSVWriterTest)

#include "tst_csvwritertest.moc"

#!/bin/sh

OUTPUT_FILE=test.xml
CK_DEFAULT_TIMEOUT=4

. ./test_vars

if [ $HAVE_FORK -eq 1 ]; then
expected_xml="<?xml version=\"1.0\"?>
<?xml-stylesheet type=\"text/xsl\" href=\"http://check.sourceforge.net/xml/check_unittest.xslt\"?>
<testsuites xmlns=\"http://check.sourceforge.net/ns\">
  <suite>
    <title>S1</title>
    <test result=\"success\">
      <fn>ex_output.c:11</fn>
      <id>test_pass</id>
      <iteration>0</iteration>
      <description>Core</description>
      <message>Passed</message>
    </test>
    <test result=\"failure\">
      <fn>ex_output.c:17</fn>
      <id>test_fail</id>
      <iteration>0</iteration>
      <description>Core</description>
      <message>Failure</message>
    </test>
    <test result=\"error\">
      <fn>ex_output.c:26</fn>
      <id>test_exit</id>
      <iteration>0</iteration>
      <description>Core</description>
      <message>Early exit with return value 1</message>
    </test>
  </suite>
  <suite>
    <title>S2</title>
    <test result=\"success\">
      <fn>ex_output.c:46</fn>
      <id>test_pass2</id>
      <iteration>0</iteration>
      <description>Core</description>
      <message>Passed</message>
    </test>
    <test result=\"failure\">
      <fn>ex_output.c:52</fn>
      <id>test_loop</id>
      <iteration>0</iteration>
      <description>Core</description>
      <message>Iteration 0 failed</message>
    </test>
    <test result=\"success\">
      <fn>ex_output.c:52</fn>
      <id>test_loop</id>
      <iteration>1</iteration>
      <description>Core</description>
      <message>Passed</message>
    </test>
    <test result=\"failure\">
      <fn>ex_output.c:52</fn>
      <id>test_loop</id>
      <iteration>2</iteration>
      <description>Core</description>
      <message>Iteration 2 failed</message>
    </test>
  </suite>
  <suite>
    <title>XML escape &quot; &apos; &lt; &gt; &amp; tests</title>
    <test result=\"failure\">
      <fn>ex_output.c:58</fn>
      <id>test_xml_esc_fail_msg</id>
      <iteration>0</iteration>
      <description>description &quot; &apos; &lt; &gt; &amp;</description>
      <message>fail &quot; &apos; &lt; &gt; &amp; message</message>
    </test>
  </suite>
</testsuites>"
expected_duration_count=9
else
expected_xml="<?xml version=\"1.0\"?>
<?xml-stylesheet type=\"text/xsl\" href=\"http://check.sourceforge.net/xml/check_unittest.xslt\"?>
<testsuites xmlns=\"http://check.sourceforge.net/ns\">
  <suite>
    <title>S1</title>
    <test result=\"success\">
      <fn>ex_output.c:11</fn>
      <id>test_pass</id>
      <iteration>0</iteration>
      <description>Core</description>
      <message>Passed</message>
    </test>
    <test result=\"failure\">
      <fn>ex_output.c:17</fn>
      <id>test_fail</id>
      <iteration>0</iteration>
      <description>Core</description>
      <message>Failure</message>
    </test>
  </suite>
  <suite>
    <title>S2</title>
    <test result=\"success\">
      <fn>ex_output.c:46</fn>
      <id>test_pass2</id>
      <iteration>0</iteration>
      <description>Core</description>
      <message>Passed</message>
    </test>
    <test result=\"failure\">
      <fn>ex_output.c:52</fn>
      <id>test_loop</id>
      <iteration>0</iteration>
      <description>Core</description>
      <message>Iteration 0 failed</message>
    </test>
    <test result=\"success\">
      <fn>ex_output.c:52</fn>
      <id>test_loop</id>
      <iteration>1</iteration>
      <description>Core</description>
      <message>Passed</message>
    </test>
    <test result=\"failure\">
      <fn>ex_output.c:52</fn>
      <id>test_loop</id>
      <iteration>2</iteration>
      <description>Core</description>
      <message>Iteration 2 failed</message>
    </test>
  </suite>
  <suite>
    <title>XML escape &quot; &apos; &lt; &gt; &amp; tests</title>
    <test result=\"failure\">
      <fn>ex_output.c:58</fn>
      <id>test_xml_esc_fail_msg</id>
      <iteration>0</iteration>
      <description>description &quot; &apos; &lt; &gt; &amp;</description>
      <message>fail &quot; &apos; &lt; &gt; &amp; message</message>
    </test>
  </suite>
</testsuites>"
expected_duration_count=8
fi

rm -f ${OUTPUT_FILE}
export CK_DEFAULT_TIMEOUT
./ex_output${EXEEXT} CK_MINIMAL XML NORMAL > /dev/null
actual_xml=`cat ${OUTPUT_FILE} | tr -d "\r" | grep -v \<duration\> | grep -v \<datetime\> | grep -v \<path\>`
if [ x"${expected_xml}" != x"${actual_xml}" ]; then
    echo "Problem with ex_xml_output${EXEEXT}";
    echo "Expected:";
    echo "${expected_xml}";
    echo "Got:";
    echo "${actual_xml}";
    exit 1;
fi

actual_duration_count=`grep -c \<duration\> ${OUTPUT_FILE}`
if [ x"${expected_duration_count}" != x"${actual_duration_count}" ]; then
    echo "Wrong number of <duration> elements in ${OUTPUT_FILE}, ${expected_duration_count} vs ${actual_duration_count}";
    exit 1;
fi

for duration in `grep "\<duration\>" ${OUTPUT_FILE} | cut -d ">" -f 2 | cut -d "<" -f 1`; do
int_duration=`echo $duration | cut -d "." -f 1`
if [ "${int_duration}" -ne "-1" ] && [ "${int_duration}" -gt "${CK_DEFAULT_TIMEOUT}" ]; then
    echo "Problem with duration ${duration}; is not valid. Should be -1 or in [0, ${CK_DEFAULT_TIMEOUT}]"
    exit 1
fi
done


exit 0

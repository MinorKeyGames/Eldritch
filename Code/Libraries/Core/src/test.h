#ifndef TEST_H
#define TEST_H

// System for generating unit test reports

#define STARTTESTS( name ) Test::GetInstance(#name "_unit_test_report.txt")
#define TEST( exp ) {Test::GetInstance()->IncrementTotal();\
	if(exp){Test::GetInstance()->IncrementSuccessful();\
	Test::GetInstance()->AddToReport(#exp,true);}else{\
	Test::GetInstance()->AddToReport(#exp,false);}}
#define TESTSFAILED Test::GetInstance()->GetNumFailed()
#define ENDTESTS Test::DeleteInstance()

class IDataStream;

class Test
{
private:
	Test( const char* Filename );
	~Test();

	static Test* m_Instance;

	unsigned int	m_TotalTests;
	unsigned int	m_SuccessfulTests;
	IDataStream*	m_Stream;

public:
	void	AddToReport( const char* Expression, bool Successful );
	void	CloseReport();
	void	IncrementTotal();
	void	IncrementSuccessful();
	int		GetNumFailed();

	static Test*	GetInstance( const char* Filename = NULL );
	static void		DeleteInstance();
};

#endif	// TEST_H
int a, b, c;
int fibo(int a)
{
	if (a == 1 || a == 2)
		return 1;
	return fibo(a - 1) + fibo(a - 2);
}

int main()
{
	int m, n, i;
	float f;
	char arr[5];
	m = read();
	m = 5;
	arr[3] = 'a';
	i = -1;
	m += 1;
	i++;
	while (i <= m)
	{
		n = fibo(i);
		write(n);		
	}  
	return 0;
}
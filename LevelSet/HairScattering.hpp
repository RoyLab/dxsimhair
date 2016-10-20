#pragma once
#include<cmath>
#include<cstdlib>
#include<vector>
#include <iostream>
#include <algorithm>

using namespace std;
const double piover180 = 0.0174532925f;
#ifdef min
#undef min
#endif
class LightScattering
{

public:
	double phi_i;
	double phi_r;
	double phi;
	double phi_h;
	double theta_i;
	double theta_r;
	double theta_h;
	double theta_d;
	double mu_1;
	double mu_2;
	double c;
	double mu = 1.55;
	double A;
	double absorption = 0;

	const double pi = 3.141592653;
	const double e = 2.7182818282135;


	const double absorptionr = 0.54;
	const double absorptiong = 0.64;
	const double absorptionb = 0.8;
	const double eccentricity = 1.0;

	const double longshift_R = -8*piover180;
	const double longshift_TT = -4*piover180;
	const double longshift_TRT = -12 * piover180;

	const double longwidth_R = 8*piover180;
	const double longwidth_TT = 4*piover180;
	const double longwidth_TRT = 16*piover180;

	const double glintscale = 0.4;
	const double azimuthalwidth = 1.5*piover180;
	const double faderange = 0.3;
	const double causticintensity = 0.5;
	double trinormal(double x)
	{
		while (!(pi >= x && x >= -pi))
		{
			if (-pi>x)x = x + 2 * pi;
			else if (x>pi) x = x - 2 * pi;
		}
		return x;
	}

	double testsolve(double x)
	{
		if (pi / 2 >= x && x >= -pi / 2)return true;
		else return false;
	}

	double Gaussian(double sdeviation, double x)
	{
		return (1.0 / e) * pow(e, -x*x / (2 * sdeviation*sdeviation));
	}

	double smoothstep(double a, double b, double x)
	{
		if (x<a)return 1;
		if (x>b)return 0;
		else return (x - b) / (a - b);
	}

	double solve(int p)
	{
		if (p == 0)
		{
			return -phi / 2;
		}
		else if (p == 1)
		{
			double a = -8.0*p*c / pow(pi, 3);
			double b = 0;
			double c = 6.0*p*c / pi - 2;
			double d = -p*pi - phi;
			double delta = pow(b*c / (6.0*a*a) - b*b*b / (27.0*a*a*a) - d / (2.0*a), 2) + pow(c / (3.0*a) - b*b / (9.0*a*a), 3);
			double root1 = 0;
			if ((b*c / (6.0*a*a) - b*b*b / (27.0*a*a*a) - d / (2.0*a) - sqrt(delta))<0)root1 = -b / (3.0*a) + pow(b*c / (6.0*a*a) - b*b*b / (27.0*a*a*a) - d / (2.0*a) + sqrt(delta), 1.0 / 3.0) - pow(abs(b*c / (6.0*a*a) - b*b*b / (27.0*a*a*a) - d / (2.0*a) - sqrt(delta)), 1.0 / 3);
			else root1 = -b / (3.0*a) + pow(b*c / (6.0*a*a) - b*b*b / (27.0*a*a*a) - d / (2.0*a) + sqrt(delta), 1.0 / 3.0) + pow(b*c / (6.0*a*a) - b*b*b / (27.0*a*a*a) - d / (2.0*a) - sqrt(delta), 1.0 / 3);
			return root1;
		}
		else
		{
			double a = -8.0*p*c / pow(pi, 3);
			double b = 0;
			double c = 6.0*p*c / pi - 2;
			double d = -p*pi - phi;
			double delta = pow(b*c / (6.0*a*a) - b*b*b / (27.0*a*a*a) - d / (2.0*a), 2) + pow(c / (3.0*a) - b*b / (9.0*a*a), 3);
			if (delta>0)return 1;
			else return 3;
		}
	}

	double solve11(double p)
	{
		double a = -8.0*p*c / pow(pi, 3);
		double b = 0;
		double c = 6.0*p*c / pi - 2;
		double d = p*pi - phi;
		double delta = pow(b*c / (6.0*a*a) - b*b*b / (27.0*a*a*a) - d / (2.0*a), 2) + pow(c / (3.0*a) - b*b / (9.0*a*a), 3);
		double root1 = 0;
		if ((b*c / (6.0*a*a) - b*b*b / (27.0*a*a*a) - d / (2.0*a) - sqrt(delta))<0)root1 = -b / (3.0*a) + pow(b*c / (6.0*a*a) - b*b*b / (27.0*a*a*a) - d / (2.0*a) + sqrt(delta), 1.0 / 3.0) - pow(abs(b*c / (6.0*a*a) - b*b*b / (27.0*a*a*a) - d / (2.0*a) - sqrt(delta)), 1.0 / 3);
		else root1 = -b / (3.0*a) + pow(b*c / (6.0*a*a) - b*b*b / (27.0*a*a*a) - d / (2.0*a) + sqrt(delta), 1.0 / 3.0) + pow(b*c / (6.0*a*a) - b*b*b / (27.0*a*a*a) - d / (2.0*a) - sqrt(delta), 1.0 / 3);
		return root1;
	}

	double solve1()
	{
		double p = 2.0;
		double a = -8.0*p*c / pow(pi, 3);
		double b = 0;
		double c = 6.0*p*c / pi - 2;
		double d = -p*pi - phi;
		double alpha = b*c / (6.0*a*a) - b*b*b / (27.0*a*a*a) - d / (2.0*a);
		double beta = c / (3.0*a) - b*b / (9.0*a*a);
		return -b / (3 * a) + 2 * sqrt(-beta)*cos(acos(alpha / pow(-beta, 3.0 / 2.0)) / 3);
	}

	double solve2()
	{
		double p = 2.0;
		double a = -8.0*p*c / pow(pi, 3);
		double b = 0;
		double c = 6.0*p*c / pi - 2;
		double d = -p*pi - phi;
		double alpha = b*c / (6.0*a*a) - b*b*b / (27.0*a*a*a) - d / (2.0*a);
		double beta = c / (3.0*a) - b*b / (9.0*a*a);
		return -b / (3 * a) + 2 * sqrt(-beta)*cos((acos(alpha / pow(-beta, 3.0 / 2.0)) + 2 * pi) / 3);
	}

	double solve3()
	{
		double p = 2.0;
		double a = -8.0*p*c / pow(pi, 3);
		double b = 0;
		double c = 6.0*p*c / pi - 2;
		double d = -p*pi - phi;
		double alpha = b*c / (6.0*a*a) - b*b*b / (27.0*a*a*a) - d / (2.0*a);
		double beta = c / (3.0*a) - b*b / (9.0*a*a);
		return -b / (3 * a) + 2 * sqrt(-beta)*cos((acos(alpha / pow(-beta, 3.0 / 2.0)) - 2 * pi) / 3);
	}

	double F(double Mu1, double Mu2, double gamma)
	{
		double R_p = 1, R_s = 1;
		if (sin(gamma) / Mu2 <= 1)R_p = pow((sqrt(1 - pow(sin(gamma) / Mu2, 2)) - Mu2*cos(gamma)) / (sqrt(1 - pow(sin(gamma) / Mu2, 2)) + Mu2*cos(gamma)), 2);
		if (R_p > 1)R_p = 1;
		if (sin(gamma) / Mu1 <= 1)R_s = pow((cos(gamma) - Mu1*sqrt(1 - pow(1 / Mu1*sin(gamma), 2))) / (1 * cos(gamma) + Mu1*sqrt(1 - pow(1 / Mu1*sin(gamma), 2))), 2);
		if (R_s > 1)R_s = 1;
		return (R_s + R_p) / 2;
	}

	double N_R()
	{
		double gamma_i = solve(0);
		double h = sin(gamma_i);
		double gamma_t = asin(h / mu_1);
		return F(mu_1, mu_2, gamma_i) / abs(-2.0 / sqrt(1 - h*h));
	}

	double N_TT()
	{
		double gamma_i = solve(1);
		double h = sin(gamma_i);
		double gamma_t = asin(h / mu_1);
		double theta_t = asin(sin(theta_i / mu));
		return (1 - F(mu_1, mu_2, gamma_i))*(1 - F(mu_1, mu_2, gamma_i))*pow(e, -(2 * absorption / cos(theta_t))*(1 + cos(2 * gamma_t))) / abs((2.0 / mu_1) / sqrt(1 - h*h / (mu_1*mu_1)) - 2.0 / sqrt(1 - h*h));
	}

	double N_TRT()
	{
		double mu1 = 2.0*(mu - 1)*eccentricity*eccentricity - mu + 2.0;
		double mu2 = 2.0*(mu - 1) / (eccentricity*eccentricity) - mu + 2.0;
		mu = (mu1 + mu2 + cos(2 * phi_h)*(mu1 - mu2)) / 2.0;
		mu_1 = abs(sqrt(mu*mu - sin(theta_d)*sin(theta_d)) / cos(theta_d));
		mu_2 = abs(mu*mu*cos(theta_d) / sqrt(mu*mu - sin(theta_d)*sin(theta_d)));
		c = asin(1.0 / mu_1);
		double ans = 0.0;
		int root = solve(2);
		if (root == 1)
		{
			double gamma_i = 0.0;
			if (testsolve(solve11(-2)))gamma_i = solve11(-2);
			else if (testsolve(solve11(0)))gamma_i = solve11(0);
			else if (testsolve(solve11(2)))gamma_i = solve11(2);
			double h = sin(gamma_i);
			double gamma_t = asin(h / mu_1);
			double theta_t = asin(sin(theta_i / mu));
			ans = (1 - F(mu_1, mu_2, gamma_i))*(1 - F(mu_1, mu_2, gamma_i))*F(1.0 / mu_1, 1.0 / mu_2, gamma_t)*pow(e, -(4 * absorption / cos(theta_t))*(1 + cos(2 * gamma_t))) / abs((2.0 / mu_1) / sqrt(1 - h*h / (mu_1*mu_1)) - 2.0 / sqrt(1 - h*h));
			A = (1 - F(mu_1, mu_2, gamma_i))*(1 - F(mu_1, mu_2, gamma_i))*F(1.0 / mu_1, 1.0 / mu_2, gamma_t)*pow(e, -(4 * absorption / cos(theta_t))*(1 + cos(2 * gamma_t)));
			//cout<<absorption<<' '<<pow(e,-(4*absorption/cos(theta_t))*(1+cos(2*gamma_t)))<<endl;
		}
		else
		{
			double gamma_i = solve1();
			double h = sin(gamma_i);
			double gamma_t = asin(h / mu_1);
			double theta_t = asin(sin(theta_i / mu));
			ans += (1 - F(mu_1, mu_2, gamma_i))*(1 - F(mu_1, mu_2, gamma_i))*F(1.0 / mu_1, 1.0 / mu_2, gamma_t)*pow(e, -(4 * absorption / cos(theta_t))*(1 + cos(2 * gamma_t))) / abs((2.0 / mu_1) / sqrt(1 - h*h / (mu_1*mu_1)) - 2.0 / sqrt(1 - h*h));

			gamma_i = solve2();
			h = sin(gamma_i);
			gamma_t = asin(h / mu_1);
			ans += (1 - F(mu_1, mu_2, gamma_i))*(1 - F(mu_1, mu_2, gamma_i))*F(1.0 / mu_1, 1.0 / mu_2, gamma_t)*pow(e, -(4 * absorption / cos(theta_t))*(1 + cos(2 * gamma_t))) / abs((2.0 / mu_1) / sqrt(1 - h*h / (mu_1*mu_1)) - 2.0 / sqrt(1 - h*h));

			gamma_i = solve3();
			h = sin(gamma_i);
			gamma_t = asin(h / mu_1);
			ans += (1 - F(mu_1, mu_2, gamma_i))*(1 - F(mu_1, mu_2, gamma_i))*F(1.0 / mu_1, 1.0 / mu_2, gamma_t)*pow(e, -(4 * absorption / cos(theta_t))*(1 + cos(2 * gamma_t))) / abs((2.0 / mu_1) / sqrt(1 - h*h / (mu_1*mu_1)) - 2.0 / sqrt(1 - h*h));
		}

		double h_c = 0.0;
		double phi_c = 0.0;
		double delta_h = 0.0;
		double t = 0.0;
		if (mu_1<2)
		{
			h_c = (4 - mu_1*mu_1) / 3;
			phi_c = 2.0*2.0*asin(h_c / mu_1) - 2.0*asin(h_c) + 2.0*pi;
			phi_c = trinormal(phi_c);
			delta_h = std::min(causticintensity, 2.0*sqrt(2.0*azimuthalwidth / abs(4 * h_c / (pow(mu_1, 3)*pow(1 - h_c*h_c / (mu_1*mu_1), 3.0 / 2.0)) - 2 * h_c / pow(1 - h_c*h_c, 3.0 / 2.0))));
			t = 1;
		}
		else
		{
			phi_c = 0;
			delta_h = causticintensity;
			t = smoothstep(2, 2 + faderange, mu_1);
		}
		ans *= (1 - t*Gaussian(azimuthalwidth, phi - phi_c) / Gaussian(azimuthalwidth, 0));
		ans *= (1 - t*Gaussian(azimuthalwidth, phi + phi_c) / Gaussian(azimuthalwidth, 0));
		ans += t*glintscale*A*delta_h*(Gaussian(azimuthalwidth, phi - phi_c) + Gaussian(azimuthalwidth, phi + phi_c));

		return ans;
	}

	double M_R()
	{
		return Gaussian(longwidth_R, (theta_h - longshift_R))  * cos(theta_i) /  cos(theta_d) / cos(theta_d);
	}

	double M_TT()
	{
		return Gaussian(longwidth_TT, theta_h - longshift_TT) * cos(theta_i) / cos(theta_d) / cos(theta_d);
	}

	double M_TRT()
	{
		return Gaussian(longwidth_TRT, theta_h - longshift_TRT) * cos(theta_i) / cos(theta_d) / cos(theta_d);
	}

public:
	LightScattering() {};
	~LightScattering() {};

	void scattering(double Phi_i, double Phi_r, double Theta_i, double Theta_r, int rgb)
	{
		phi_i = Phi_i;
		phi_r = Phi_r;
		phi = phi_r - phi_i;
		phi_h = (phi_r + phi_i) / 2;
		theta_i = Theta_i;
		theta_r = Theta_r;
		theta_h = (theta_i + theta_r) / 2;
		theta_d = (theta_r - theta_i) / 2;
		if (rgb == 0)absorption = absorptionr;
		else if (rgb == 1)absorption = absorptiong;
		else if (rgb == 2)absorption = absorptionb;
		mu_1 = abs(sqrt(mu*mu - sin(theta_d)*sin(theta_d)) / cos(theta_d));
		mu_2 = abs(mu*mu*cos(theta_d) / sqrt(mu*mu - sin(theta_d)*sin(theta_d)));
		c = asin(1.0 / mu_1);
		//std::cout << M_R() << ' ' << M_TRT() << std::endl;
		//return M_R()*N_R() / (cos(theta_d)*cos(theta_d)) * 2 +
		//	M_TT()*N_TT() / (cos(theta_d)*cos(theta_d)) * 2 +
		//	M_TRT()*N_TRT() / (cos(theta_d)*cos(theta_d)) * 250;
	}
};
#pragma once

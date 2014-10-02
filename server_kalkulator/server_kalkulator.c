/* Contoh Sederhana Penggunaan Libevent untuk membuat server kalkulator 
*yang di input/output menggunakan HTML
created by http://deeprhezy.tumblr.com
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/keyvalq_struct.h>


static void parse_and_answer(struct evkeyvalq kv, char answer[]);

/*Callback ini akan dipanggil ketika kita mendapatkan permintaan http yang tidak cocok
  * Setiap callback lainnya. Seperti callback server yang evhttp, ia memiliki pekerjaan sederhana:
  * Akhirnya harus memanggil evhttp_send_error () atau evhttp_send_reply ().*/

static void
send_document_cb (struct evhttp_request *req, void *arg)
{
	struct evbuffer *evb = NULL;
	const char *uri = evhttp_request_get_uri (req);
	struct evhttp_uri *decoded = NULL;

	char answer[256];

	/*Fungsi menghandle permintaan POST. */
	if (evhttp_request_get_command (req) != EVHTTP_REQ_POST)
	{
		evhttp_send_reply(req, 200, "OK", NULL);
		return;
	}

	/* Decode URL */
	decoded = evhttp_uri_parse (uri);
	if (! decoded)
	{
		printf ("It's not a good URI. Sending BADREQUEST\n");
		evhttp_send_error (req, HTTP_BADREQUEST, 0);
		return;
	}

	/* Decode payload */
	struct evkeyvalq kv;
	memset (&kv, 0, sizeof (kv));
	struct evbuffer *buf = evhttp_request_get_input_buffer (req);
	evbuffer_add (buf, "", 1);    /* NUL-terminate the buffer */
	char *payload = (char *) evbuffer_pullup (buf, -1);

	if (0 != evhttp_parse_query_str (payload, &kv))
	{
		printf ("Malformed payload. Sending BADREQUEST\n");
		evhttp_send_error (req, HTTP_BADREQUEST, 0);
		return;
	}

	/*  Memberikan pesan penguraian. */
	printf("\n\n%s\n\n", payload);

	parse_and_answer(kv, answer);

	evhttp_clear_headers (&kv);   

	/* Membuat fungsi baru evbuffer */
	evb = evbuffer_new ();

	/* Fungsi mempersiapkan jawaban dengan format HTML yang dimulai dengan Header */
	evhttp_add_header (evhttp_request_get_output_headers (req), "Content-Type", "text/html;");

	/* Kemudian menambahkan pesan*/
	evbuffer_add (evb, answer, strlen (answer));

	/* Mengirimkan pesan kembali ke pengirim. */
	evhttp_send_reply (req, 200, "OK", evb);

	/* Membersihkan memori dari evhttp dan evbuffer */
	if (decoded)
		evhttp_uri_free (decoded);
	if (evb)
		evbuffer_free (evb);

}

/* Fungsi sederhana dari Kalkulator
 * Setelah diproses akan di tampilkan kebrowser dan terminal.
 */
static void parse_and_answer(struct evkeyvalq kv, char answer[])
{
	const char* nama;
	const char* angka1;
	const char* angka2;	
	const char* operator;
	float angka1x;
	float angka2x;
	float hasil;

	/* Mengekstrak permintaan dengan header dan membuatkan variabel baru*/
	nama = evhttp_find_header (&kv, "nama");
	angka1 = evhttp_find_header (&kv, "angka1");
	angka2 = evhttp_find_header (&kv, "angka2");
	operator = evhttp_find_header (&kv, "operator");

	/* Mencetak ke terminal/konsul */
	printf("Nama : %s\n", nama);
	printf("Angka 1 : %s\n", angka1);
	printf("Angka 2 : %s\n", angka2);
	printf("Operator :%s\n", operator);

	angka1x=atof(angka1);
	angka2x=atof(angka2);

	if(strcmp(operator,"tambah")==0)
	{
		hasil=angka1x+angka2x;
		operator="+";
	}
	else if(strcmp(operator,"kurang")==0)
	{
		hasil=angka1x-angka2x;
		operator="-";
	}
	else if(strcmp(operator,"kali")==0)
	{
		hasil=angka1x*angka2x;
		operator="x";
	}
	else if(strcmp(operator,"bagi")==0)
	{
		hasil=angka1x/angka2x;
		operator=":";
		//hasil=ceilf(hasilx*100)/100;
	}
	
	printf("Hasil %.0f %s %.0f=%.2f\n",angka1x,operator,angka2x,hasil);

	/* Mencetak hasilnya ke browser*/
	evutil_snprintf (answer,256,"Hai %s </br> Hasil %.0f%s%.0f = %.2f\n",nama,angka1x,operator,angka2x,hasil);
}



/*
 * Fungsi pertama yang menciptakan loop dan mengikat socket ke alamat yang kita siapkan.
 * Kemudian akan mulai infinite loop, menunggu penerimaan pesan sehingga dapat memanggil callback.
 */
int main (int argc, char **argv)
{
	struct event_base *base;
	struct evhttp *http;
	struct evhttp_bound_socket *handle;

	unsigned short port = 8080;

	base = event_base_new ();
	if (! base)
	{
		fprintf (stderr, "Couldn't create an event_base: exiting\n");
		return 1;
	}

	/* Membuat objek evhttp baru bernama http. */
	http = evhttp_new (base);
	if (! http)
	{
		fprintf (stderr, "couldn't create evhttp. Exiting.\n");
		return 1;
	}

	/* Ini adalah callback yang akan dipanggil ketika permintaan masuk */
	evhttp_set_gencb (http, send_document_cb, NULL);

	/* Sekarang kita membuat evhttp port untuk mendengarkan pada url localhost  */
	handle = evhttp_bind_socket_with_handle (http, "127.0.0.1", port);
	if (! handle)
	{
		fprintf (stderr, "couldn't bind to port %d. Exiting.\n", (int) port);
		return 1;
	}

	/* Loop akan diulang sampai terdapat sebuah respon */
	event_base_dispatch (base);
	
	return 0;
}

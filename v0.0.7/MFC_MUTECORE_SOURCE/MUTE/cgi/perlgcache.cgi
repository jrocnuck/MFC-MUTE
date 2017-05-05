#!/usr/bin/perl -wT
use strict;

# Copyright (c) 2003 Jon Atkins http://www.jonatkins.com/
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.



my $cacheversion = "0.6.4";

###### options ######

# this is added to the start of all files loaded/saved
# NOTE: can be a full path or a relative path
# NOTE: the cache MUST have write-access to the specified directory
# eg. my $prefix = "/tmp/gcache.example.com" would store files in "/tmp/gcache.example.com-*"
# eg. my $prefix = "data/data" would store files in a data directory below the location of the CGI script
my $prefix = "../cgi-data/gwebcache/data"; # will use "data-*" in the same directory as this cgi script


# a large number of URLs are stored, and clients get a random selection
# returned on each request.
# URLs are removed from the list once they are $maxurlage minutes old
# if URL testing is enabled (see $checkurls below) then caches
# are tested with a ping request. failed caches are also stored to prevent
# repeated retests on additional updates

# return this many alternate cache urls
my $maxurlret = 10;	# suggested range: between 10 and 20
# store this many URLs
my $maxurlstore = 200;	# suggested range: 200 to 500
# remove URLs after this many minutes
my $maxurlage = 1*24*60;	# in minutes - default 1 day

# number of hosts to keep and return
my $maxhost = 20;
# maximum age of a host entry, in minutes
my $maxhostage = 6*60;		# in minutes - default 12 hours by default


# set to non-zero to block clients which don't send a client and version parameter
# (this setting only applys to v1 requests - v2 requests require a client string)
# NOTE: in the original cache specs sending client/version was optional
my $blockv1anon = 0;

# log cache stats
my $enablestats = 1;

# minimum time, in minutes, between updates
my $updatelimit = 55;	# suggested range: 30 - 55 minutes

# check each URL submitted works
my $checkurls = 0;

# HTTP proxy server to use when $checkurls is 1 - undef for no proxy
my $proxyhost = undef;
my $proxyport = undef;

# request rate limiting
# set either to zero to disable this feature
my $requestlimitcount = 0;	# suggested value: 0, or 5 - 10
my $requestlimittime = 55;	# suggested maximum 120 (2 hours)


# should we answer old-style v1 requests?
my $enablev1requests = 1;


# EXPERIMENTAL: see the notes at http://www.jonatkins.com/perlgcache/
# before enabling this feature.
my $enablev2requests = 0;


# note: if both v1/v2 are set to 0, act as if v1 is enabled




###### end of configuration ######
# NO USER OPTIONS AFTER HERE


use IO::Socket;		# for testing of sent URLs


###### global vars #####

# globals: read time once - to avoid possible race conditions
# note: values set at the top of dorequest() - ready for FastCGI version
my $time;
my $day;


###### subroutines ######

# first, generic routines...

sub error
{
	my ( @args ) = @_;

	print "ERROR: ".(join ' ', @args)."\n";
	exit;
}

sub paramdecode
{
	my ( $query ) = @_;

	my %param;

	return () unless defined $query;

	foreach my $arg ( split /&/, $query )
	{
		my ( $name, $val ) = split /=/, $arg;

		next unless defined $name and defined $val;

		$val =~ s/\+/ /g;
		$val =~ s/%([0-9a-fA-F]{2})/chr hex $1/eg;

		$param{$name} = $val;
	}

	return %param;
}

sub loadarrayhash
{
	my ( $filename ) = @_;

	my @array;

	open IN, "$filename" or return ();

	my $header = <IN>;
	return () unless defined $header;

	chomp $header;

	my @fields = split /\|/, $header;

	foreach my $line ( <IN> )
	{
		chomp $line;
		my @items = split /\|/, $line, scalar @fields;

		my %item;
		for ( my $i=0; $i < @items; $i++ )
		{
			$item{$fields[$i]} = defined $items[$i] ? $items[$i] : '';
		}

		push @array, \%item;
	}
	close IN;

#print Dumper ( \@array );

	return @array;
}

sub savearrayhash
{
	my ( $filename, @array ) = @_;

#TODO: proper error checking in here, so we don't loose data files when the
#disk is full
# (build the file in a string then write in one go, to make this easier..?
#  may be better on file I/O performance this way)


	my $tmpfile = "$filename.tmp.$$";
	open OUT, "> $tmpfile" or die;

	return if @array == 0;

	my @fields = sort keys %{$array[0]};

	print OUT join "|", @fields;
	print OUT "\n";


	foreach my $item ( @array )
	{
		my $first = 1;
		foreach my $field ( @fields )
		{
			print OUT "|" unless $first;
			$first = 0;

			my $val = ${$item}{$field};
			$val = '' unless defined $val;

			print OUT "$val";
		}
		print OUT "\n";
	}

	close OUT;

	rename $tmpfile, $filename or error "rename of $tmpfile to $filename failed";

}


sub loadhashhash
{
	my ( $filename ) = @_;

	# right - let's get loadarrayhash to do the work here...

	my @array = loadarrayhash $filename;

	my %hash;

	foreach my $item ( @array )
	{
		my $key = ${$item}{KEY};

		$hash{$key} = $item;
		delete $hash{$key}{KEY};

	}

#print Dumper \%hash;

	return %hash;
}

sub savehashhash
{
	my ( $filename, %hash ) = @_;

#print Dumper \%hash;

	my @array;

	foreach my $item ( keys %hash )
	{
		my %newitem;
		%newitem = %{$hash{$item}};
		$newitem{KEY} = $item;
		push @array, \%newitem;
	}

	savearrayhash $filename, @array;
}


sub fixarrayfields
{
	my ( $fieldsref, $arrayref ) = @_;

	foreach my $item ( @{$arrayref} )
	{
		# add any missing fields...
		foreach my $field ( keys %{$fieldsref} )
		{
			${$item}{$field} = ${$fieldsref}{$field} unless defined ${$item}{$field};
		}
		# .. and remove any not required...
		foreach my $field ( keys %{$item} )
		{
			delete ${$item}{$field} unless exists ${$fieldsref}{$field};
		}
	}

}


sub getiptype
{
	my ( $ip ) = @_;

	#TODO: add support for IPv6 here...

	return "invalid" unless $ip =~ m/^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$/;

	my ( $a, $b, $c, $d ) = ( $1, $2, $3, $4 );

	return "invalid" if ( $a<0 or $a>255 or $b<0 or $b>255 or $c<0 or $c>255 or $d<0 or $d>255 );

	# don't allow 0.*.*.* (this net), 127.*.*.* (loopback) or 224-255.*.*.* (multicast, class-e, broadcast)
	return "unusable" if ( $a==0 or $a==127 or $a>=224 );

	# mark 10.*.*.* (private a), 169.254.*.* (local link), 172.16-31.*.* (private b), 192.0.2.* (test net), 192.168.*.* (private c)
	return "private" if ( ($a==10) or ($a==169 and $b==254) or ($a==172 and ($b>=16 and $b<=31)) or ($a==192 and $b==0 and $c==2) or ($a==192 and $b==168) );

	return "public";
}


# HTTP request routines - for testing of URLs....
# used for the internals of http requests...
sub socket_send_and_get
{
	my ( $host, $port, @data ) = @_;
	my ( $remote, $recv );

	$remote = IO::Socket::INET->new ( Proto => 'tcp', PeerAddr => $host, PeerPort => $port, Timeout => 60 );
	return undef unless $remote;

	$remote->autoflush(1);

	print $remote @data;
	local $/ = undef;

	$recv = <$remote>;
	close $remote;

	alarm 0;

	return $recv;
}

# sends a http request to the given url and returns all data (header and body) as a single string.
sub http_request
{
	my ( $url ) = @_;

	if ( $url =~ m{^http://([a-zA-Z0-9-.]+)(?::(\d{1,5}))?(.*)$} )
	{
		my $host = $1;
		my $port = $2;
		my $path = $3;

		$port = 80 unless defined $port;
		$path = '/' if $path eq '';

		if ( defined $proxyhost and defined $proxyport )
		{
			my $reply = socket_send_and_get $proxyhost, $proxyport, "GET $url HTTP/1.0\r\n", "Host: $host:$port\r\n", "User-agent: perlgcache/$cacheversion (via proxy)\r\n", "X-RequestingIP: $ENV{REMOTE_ADDR}\r\n", "\r\n";
			return $reply;
		}

		my $reply = socket_send_and_get $host, $port, "GET $path HTTP/1.0\r\n", "Host: $host:$port\r\n", "User-agent: perlgcache/$cacheversion\r\n", "X-RequestingIP: $ENV{REMOTE_ADDR}\r\n", "\r\n";
		return $reply;
	}
	return undef;
}

# does a http request for a text/* object, processes redirect as required
# splits the output into an array of lines (without line terminators) and returns an array of:
# ( $code, $redircount, @lines )
sub do_http_text_request
{
	my ( $url, $redircount ) = @_;

	$redircount = 0 unless defined $redircount;

	# to stop redirect loops...
	return ( -4, $redircount ) if $redircount >= 10;


	my $reply = http_request $url;

	return ( -1, $redircount ) unless defined $reply;

	# HTTP headers should use \r\n to seperate lines - but for robustness clients can accept any format
	# i'll add that here later maybe
	my ( $header, $body ) = split /\r\n\r\n/, $reply, 2;
	return ( -5, $redircount ) unless defined $header and defined $body;
	my @header = split /\r\n/, $header;

	my $statusline = shift @header;
	return ( -2, $redircount ) unless defined $statusline;

	return ( -3, $redircount ) unless ( $statusline =~ m{^HTTP/\d+\.\d+ +(\d\d\d) *(.*)$} );
	my ( $code, $message ) = ( $1, $2 );

	if ( $code =~ m/^3\d\d$/ )
	{
		# we've got a redirect - find the Location: header and follow it...
		foreach my $headerline ( @header )
		{
			if ( $headerline =~ m{^location: +(.*)$}i )
			{
				# right - location headers *should* be absolute, but sometimes they're not...
				my $newurl = $1;

				if ( $newurl =~ m{^http:} )
				{
					# full path - no problem
					return do_http_text_request ( $newurl, $redircount+1 );
				}
				elsif ( $newurl =~ m{^/} )
				{
					# server relative url - add current server and port
					$url =~ m{^(http://.*?)/} or die;
					$newurl = $1.$newurl;

					return do_http_text_request ( $newurl, $redircount+1 );
				}
				else
				{
					# directory relative url - add current server and path
					$url =~ m{^(http:.*/)} or die;
					$newurl = $1.$newurl;

					return do_http_text_request ( $newurl, $redircount+1 );
				}
			}
		}

		# if we get here, there was no Location header...
	}

	if ( $code ne '200' )
	{
		# bad code - fail
		return ( $code, $redircount );
	}

	# right - we've got the data - now let's check the type...
	foreach my $headerline ( @header )
	{
		if ( $headerline =~ m/^content-type: +(.*$)/i )
		{
			my $type = $1;

			# error on non text/* content-types...
			error ( -6, $redircount ) unless ( $type =~ m{^text/.*$} );
		}

		# (if we don't find a content-type, just assume it's text)
	}

	# it's good - now we need to sort out the body lines....
	# fun, because they could be seperated with just '\n', '\r\n' or even '\r' (and '\n\r'?)
	my @lines;

	# (isn't perl great!!)
	@lines = split /\r\n|\n\r|\n|\r/, $body;

	return ( $code, $redircount, @lines );
}


# now, utility routines for a gwebcache...


my %hostsfields =
(
	ipport	=> '0.0.0.0:0',	# well we have to have something in there by default
	client	=> 'unknown',
	time	=> 0,
);

sub gethosts
{
	my ( $network ) = @_;

	my $file = "$prefix-hosts".($network eq 'gnutella' ? "" : "-$network");

	my @hosts = loadarrayhash $file;

	fixarrayfields \%hostsfields, \@hosts;

	# TODO: remove old/bad entries

	return @hosts;
}

sub addhost
{
	my ( $network, $ipport, $client ) = @_;

	my $file = "$prefix-hosts".($network eq 'gnutella' ? "" : "-$network");

	my @hosts = loadarrayhash $file;

	fixarrayfields \%hostsfields, \@hosts;

	my $alreadyhave = 0;
	foreach my $loopipport ( @hosts )
	{
		if ( ${$loopipport}{ipport} eq $ipport )
		{
			$alreadyhave = 1;
		}
	}


	# just return if we already have it
	# NOTE: we should probably update it's timestamp, but it isn't really worth it
	return if $alreadyhave;

	# add entry to start of list
	unshift @hosts, { ipport => $ipport, client => $client, time => $time };

	# remove extra hosts
	splice @hosts, $maxhost if @hosts > $maxhost;

	# and max age
	while ( scalar @hosts and $hosts[scalar @hosts - 1]{time} < $time - $maxhostage*60 )
	{
		pop @hosts;
	}

	savearrayhash $file, @hosts;
}

#sub pickhosts
#{
#	my ( $client, @hosts ) = @_;
#
#	$client = 'no such client' if $client eq 'unknown';
#
#	my %done;
#
#	my @newhosts;
#
#	# first, pick hosts of the same client type...
#	foreach my $item ( @hosts )
#	{
#		next if ${$item}{client} ne $client;
#		next if ${$item}{time} < $time - $maxhostage*60;
#
#		push @newhosts, $item;
#		$done{${$item}{ipport}} = 1;
#		last if @newhosts >= $maxhostret * $hostsameclientratio;
#	}
#
#	# now pick some more hosts - client not important (could be more of the same client)
#	foreach my $item ( @hosts )
#	{
#		next if defined $done{${$item}{ipport}};
#		next if ${$item}{time} < $time - $maxhostage*60;
#
#		push @newhosts, $item;
#		last if @newhosts >= $maxhostret;
#	}
#
#	return @newhosts;
#}


my %urlfields =
(
	url	=> 'http://localhost/ignore',
	time	=> 0,
	version => 'unknown',
);



sub geturls
{
	my ( $cachetype, $network ) = @_;

	my $file = "$prefix-urls".($cachetype == 1 ? "" : "-$cachetype");

	$file .= "-$network" if defined $network;


	my @urls = loadarrayhash $file;

	fixarrayfields \%urlfields, \@urls;

	# first, remove any URLs which are too old...
	# NOTE: URLs are stored newest to oldest, so we can just repeatedly drop the last one
	while ( @urls > 1 and $urls[@urls-1]{time} < $time - $maxurlage*60 )
	{
		splice @urls, -1;	# remove last entry
	}

	return @urls;
}

sub addurl
{
	my ( $cachetype, $url, $network ) = @_;

	my $file = "$prefix-urls".($cachetype == 1 ? "" : "-$cachetype");

	$file .= "-$network" if defined $network;

	my @urls = loadarrayhash $file;

	fixarrayfields \%urlfields, \@urls;

	# first, remove any URLs which are too old...
	# NOTE: URLs are stored newest to oldest, so we can just repeatedly drop the last one
	while ( @urls > 1 and $urls[@urls-1]{time} < $time - $maxurlage*60 )
	{
		splice @urls, -1;	# remove last entry
	}

	my $alreadyhave = 0;
	foreach my $loopurl ( @urls )
	{
		if ( ${$loopurl}{url} eq $url )
		{
			$alreadyhave = 1;
		}
	}

	# just return if we already have it
	# NOTE: we should really update it's timestamp, but it isn't really worth it
	return if $alreadyhave;

	my $urlcachever = 'unknown';
	if ( $checkurls )
	{
		my $requrl = $url;
		if ( $cachetype == 1 )
		{
			$requrl .= "?client=TEST&version=pgc-$cacheversion&ping=1";
		}
		else
		{
			# sending a ping isn't defined in the current GWC2 specs - so send a
			# get as well to ensure we trigger the correct mode of a dual-version cache
			$requrl .= "?client=TESTpgc-$cacheversion&ping=1&get=1";
			$requrl .= "&net=$network" if defined $network;
		}

		my ( $code, undef, @lines ) = do_http_text_request $requrl;

		if ( $code ne '200' )
		{
			$urlcachever = 'FAILED';
		}
		else
		{
			if ( $cachetype == 1 )
			{
				# v1 test - at least one line, and the first line starts with PONG
				if ( @lines >= 1 and $lines[0] =~ m{^PONG\s*([a-zA-Z0-9. _/+-]{0,100})} )
				{
					$urlcachever = $1 if $1 ne '';	# ignore if blank
				}
				else
				{
					$urlcachever = 'FAILED';
				}
			}
			else
			{
				# v2 test - each line must be of the form '[a-zA-Z]|.*'

				$urlcachever = 'FAILED' if @lines == 0;

				foreach my $line ( @lines )
				{
					chomp $line;

					if ( $line =~ m{^[iI]\|[pP][oO][nN][gG]\|([a-zA-Z0-9. _/+-]{0,100})} )
					{
						$urlcachever = $1 if $1 ne '';
					}
					elsif ( $line !~ m/^[a-zA-Z0-9]\|/ )
					{
						$urlcachever = 'FAILED';
						last;
					}
				}
			}
		}

	}

	# add entry
	unshift @urls, { url => $url, time => $time, version => $urlcachever };

	# remove extra urls
	splice @urls, $maxurlstore if @urls > $maxurlstore;

	savearrayhash $file, @urls;

}

sub pickurls
{
	my ( @urls ) = @_;

	my @newurls;

	while ( @urls >= 1 and @newurls < $maxurlret )
	{
		my $rand = int ( rand @urls );

		my $item = splice @urls, $rand, 1;	# pick an item from the list and remove it

		next if ${$item}{version} eq 'FAILED';	# ignore failed URLs

		push @newurls, $item;
	}

	return @newurls;
}


sub getnetworks
{
	my %networks = ();

	# get the networks from the hosts files...
	foreach my $file ( <$prefix-hosts*> )
	{
# nasty hack to allow for no -gnutella suffix special case
		$file = "$prefix-hosts-gnutella" if $file eq "$prefix-hosts";

		$networks{substr $file, length "$prefix-hosts-"} = 1;
	}

	# .. and the url files - for the gwc2 network only
	foreach my $file ( <$prefix-urls-2-*> )
	{
# nasty hack not needed - we always append the network name for gwc2 url databases
#		$file = "$prefix-urls-gnutella" if $file eq "$prefix-urls";

		$networks{substr $file, length "$prefix-urls-2-"} = 1;
	}

	return sort keys %networks;
}


sub statlog
{
	return unless $enablestats;

	my ( $day, $action ) = @_;
	# NOTE: loadhashhash REQUIRES that all of the hashes have the same members, or data WILL NOT be saved correctly,
	# so use this to check...
	my %stattypes =
	(
		update => 1,
		hostfile => 1,
		urlfile => 1,
		statfile => 1,
		ping => 1,

		badrequest => 1,
		block_requestrate => 1,
		block_anon => 1,
		update_block_badport => 1,
		update_block_badip => 1,
		update_block_ratelimit => 1,

		block_badpath => 1,

# v2 actions are any combination of get, ping and update - always sorted
		v2_get => 1,
		v2_ping => 1,
		v2_update => 1,
		v2_get_ping => 1,
		v2_get_update => 1,
		v2_ping_update => 1,
		v2_get_ping_update => 1,

		v2_block_anon => 1,
		v2_update_block_badport => 1,
		v2_update_block_badip => 1,
		v2_update_block_badurl => 1,
		v2_update_block_ratelimit => 1,

	);

	# make sure this stat type exists...
	return unless defined $stattypes{$action};

	my %stats = loadhashhash "$prefix-stats";

	# add in this stat item...
	$stats{$day}{$action} = 0 unless defined $stats{$day}{$action};
	$stats{$day}{$action}++;

	# and also into the totals...
	$stats{'total'}{$action} = 0 unless defined $stats{$day}{$action};
	$stats{'total'}{$action}++;

	foreach my $statday ( keys %stats )
	{
		# now remove old stat data...
		if ( $statday ne 'total' and $statday < $day - 28 )
		{
			delete $stats{$statday};
			next;
		}

		# remove bogus stats...
		foreach my $stattype ( keys %{$stats{$statday}} )
		{
			delete $stats{$statday}{$stattype} unless defined $stattypes{$stattype};
		}
		# and add unset stats...
		foreach my $stattype ( keys %stattypes )
		{
			$stats{$statday}{$stattype} = 0 unless defined $stats{$statday}{$stattype};
		}
	}

	savehashhash "$prefix-stats", %stats;
}

# returns an array of ( $client, $version ) from a decoded cgi parameter hash
# allows for both v1 client=XXX&version=a.b.c and v2 client=XXXXa.b.c style versions...
sub getclientandversion
{
	my ( %p ) = @_;

	# the V2 specs say client and version should be combined into a single parameter - work around that here...
	if ( defined $p{client} and !defined $p{version} and $p{client} =~ m/^([A-Za-z0-9]{4})(.+)$/ )
	{
		$p{client} = $1;
		$p{version} = $2;
	}

	my $client;
	if ( !defined $p{client} )
	{
		$client = "unknown";
	}
	elsif ( $p{client} =~ m/^[a-zA-Z0-9.+-]{1,20}$/ )
	{
		$client = $p{client};
	}
	else
	{
		$client = "unknown_bad";
	}

	my $version;
	if ( !defined $p{version} )
	{
		$version = "unknown";
	}
	elsif ( $p{version} =~ m{^[a-zA-Z0-9/.+ @\-]{1,40}$} )
	{
		$version = $p{version};
	}
	else
	{
		$version = "unknown_bad";
	}

	return ( $client, $version );
}


###### main code ######


# some subs, to break up the code a little...

sub dohtmlpage
{
	my ( %param ) = @_;

	$| = 1;

	my $browse = "index";
	$browse = $param{browse} if defined $param{browse} and $browse =~ m/^[a-zA-Z0-9]{1,100}$/;

	print "Content-type: text/html\n";
	print "\n";

print <<END;
<html><head><title>perlgcache $cacheversion</title></head>
<style>
body
{
	background: #fff;
	color: #000;
	font-family: "Ariel", sans-serif;
}
table th
{
	background: #ddd;
	padding: 0 8px 0 8px;
	font-size: 14px;
}
table td
{
	background: #eee;
	font-size: 12px;
}
table tr.clienttotal td
{
	color: #888;
}
hr
{
	border: solid;
	color: #ccc;
}
table td.num, table th.num
{
	text-align: right;
	padding: 2px;
	font-size: 12px;
}

</style>
<body>
<h1>perlgcache $cacheversion</h1>

<p>[
<a href="?browse=index">home</a> |
<a href="?browse=hosts">hosts</a> |
<a href="?browse=urls">urls</a> |
<a href="?browse=limited">limited IPs</a> |
<a href="?browse=stats">stats</a>
]</p>
END

	# right - it's a good idea to test the data directory is writable here - let's create then delete a file
	my $ok = 0;
	my $tmpfile = "$prefix-writetest.$$.tmp";
	if ( open OUT, "> $tmpfile" )
	{
		close OUT;

		unlink $tmpfile;

		$ok = 1;
	}

	if ( ! $ok )
	{
		print "<hr />\n";
		print "<h2>Datafile write failure</h2>\n";
		print "<p><b>This cache is not correctly configured!</b></p>\n";
		print "<p>You need to configure \$prefix to point to a writable directory.</p>\n";
		print "<hr />\n";
	}

	if ( $browse eq 'stats' )
	{
	    if ( $enablestats )
	    {
		my %stats = loadhashhash "$prefix-stats";

		print "<table><caption>Cache stats</caption>\n";
		print "<tr><td></td>\n";
		foreach my $stattype ( sort keys %{$stats{'total'}} )
		{
			next if $stats{'total'}{$stattype} == 0;

			my $statdisp = $stattype;
			$statdisp =~ s/_/ /g;

			print "<th>$statdisp</th>";
		}
		print "<td></td><th>Total</th>";
		print "</tr>\n";

		foreach my $statday ( sort keys %stats )
		{
			print "<tr><td></td></tr>\n" if $statday eq 'total';

			my $daytext = "Total";
			$daytext = scalar gmtime ($statday*60*60*24) unless $statday eq 'total';
			$daytext =~ s/00:00:00 //;
			$daytext =~ s/ /&nbsp;/g;

			print "<tr><th>$daytext</th>";

			my $total = 0;
			foreach my $stattype ( sort keys %{$stats{'total'}} )
			{
				next if $stats{'total'}{$stattype} == 0;

				print "<td class=num>$stats{$statday}{$stattype}</td>";

				$total += $stats{$statday}{$stattype};
			}
			print "<td></td><td class=num>$total</td>";
			print "</tr>\n";
		}
		print "</table>\n";
	    }

	}
	elsif ( $browse eq 'hosts' )
	{
		foreach my $network ( getnetworks )
		{
			my @hosts = gethosts $network;

			print "<table><caption>Host cache - net=$network</caption>\n";
			print "<tr><th>IP:Port</th><th>Client</th><th>Time</th></tr>\n";
			foreach my $item ( @hosts )
			{
				next if ${$item}{time} < $time - $maxhostage*60;

				print "<tr><td>${$item}{ipport}</td><td>${$item}{client}</td><td>".(scalar gmtime ${$item}{time})."</td></tr>\n";
			}
			print "<tr><td colspan=3>no hosts</td></tr>\n" if scalar @hosts == 0;
			print "<tr><td colspan=3>Maximum of $maxhost hosts stored/returned.<br>\n";
			print "Hosts are no older than $maxhostage minutes</tr></td>\n";
			print "</table>\n";
		}
	}
	elsif ( $browse eq 'urls' )
	{
		my @urls = geturls 1;

		if ( $enablev1requests )
		{
			print "<table><caption>URL cache - GWebcache</caption>\n";
			print "<tr><th>URL</th><th>Version</th><th>Time</th></tr>\n";
			foreach my $item ( @urls )
			{
				print "<tr><td><a href=\"${$item}{url}\">${$item}{url}</a></td><td>${$item}{version}</td><td>".(scalar gmtime ${$item}{time})."</td></tr>\n";
			}
			print "<tr><td colspan=3>no urls</td></tr>\n" if scalar @urls == 0;
			print "</table>\n";
		}

		if ( $enablev2requests )
		{
			foreach my $network ( getnetworks )
			{
				my @urls = geturls 2, $network;		##loadarrayhash "$prefix-urls";

				print "<table><caption>URL cache - GWebcache2 - net=$network</caption>\n";
				print "<tr><th>URL</th><th>Version</th><th>Time</th></tr>\n";
				foreach my $item ( @urls )
				{
					print "<tr><td><a href=\"${$item}{url}\">${$item}{url}</a></td><td>${$item}{version}</td><td>".(scalar gmtime ${$item}{time})."</td></tr>\n";
				}
				print "<tr><td colspan=3>no urls</td></tr>\n" if scalar @urls == 0;
				print "</table>\n";
			}
		}
	}
	elsif ( $browse eq 'limited' )
	{
		my %updateip = loadhashhash "$prefix-updateip";

		print "<table><caption>Recent updates</caption>\n";
		print "<tr><th>Client IP</th><th>Update at</th></tr>\n";
		foreach my $ip ( sort { $updateip{$b}{time} <=> $updateip{$a}{time} } keys %updateip )
		{
			next if $updateip{$ip}{time} < $time - $updatelimit*60;

			print "<tr><td>$ip</td><td>".(scalar gmtime $updateip{$ip}{time})."</td></tr>\n";
		}
		print "<tr><td colspan=2>No updates for another $updatelimit minutes</td></tr>\n";
		print "<tr><td colspan=2>no blocked IPs</td></tr>\n" if scalar keys %updateip == 0;
		print "</table>\n";

		if ( $requestlimitcount and $requestlimittime )
		{
			my %requestip = loadhashhash "$prefix-requestip";

			print "<table><caption>Recent requests</caption>\n";
			print "<tr><th>Client IP</th><th>count</th><th>time</th></tr>\n";
			my %lowcount = ();
			foreach my $ip ( sort { $requestip{$b}{count} <=> $requestip{$a}{count} } keys %requestip )
			{
				next if $requestip{$ip}{time} < $time - $requestlimittime*60;

				if ( $requestip{$ip}{count} <= $requestlimitcount )
				{
					$lowcount{$requestip{$ip}{count}}++;
				}
				else
				{
					print "<tr><td>$ip</td><td class=num>$requestip{$ip}{count}</td><td>".(scalar gmtime $requestip{$ip}{time})."</td></tr>\n";
				}
			}
			foreach my $count ( sort { $b <=> $a } keys %lowcount )
			{
				print "<tr><td colspan=3>$lowcount{$count} IP(s) have made $count request(s) in the last $requestlimittime minutes</td></tr>\n";
			}
			print "<tr><td colspan=3>No more than $requestlimitcount requests every $requestlimittime minutes</td></tr>\n";
			print "</table>\n";
		}
	}
	else
	{
print <<END;
<p>Welcome to this <a href="http://www.gnutella.com/">gnutella</a>
<a href="http://www.gnucleus.com/gwebcache/">web cache</a>.</p>


<hr />
<p>
<a href="http://www.jonatkins.com/perlgcache/">perlgcache</a>
by Jon Atkins

</p>

<iframe src="http://www.jonatkins.com/perlgcache/version/$cacheversion" width="500" height="100" frameborder=0>
Version check available <a href="http://www.jonatkins.com/perlgcache/version/$cacheversion">here<a>.
</iframe>

END
	}


	print "<p>Current time: ".(scalar gmtime $time)."</p>\n";


print <<END;
</body>
</html>
END

}




sub dov1request
{
	my ( %param ) = @_;


	my ( $client, $version ) = getclientandversion %param;

	my $network = 'gnutella';	## NOTE: default net name - supplying no net= param is the same as sending this value
##	$network = lc $1 if defined $param{net} and $param{net} =~ m/^([a-zA-Z0-9._-]{1,20})$/;
##	# (NOTE: above restrictions are the same as I use for the client string - no particular reason for this)


	if ( $blockv1anon )
	{
#		unless ( defined $param{client} and defined $param{version} )
		if ( $client =~ m/^unknown/ )	# just check for client
		{
			statlog $day, "block_anon";
			error "anonymous clients not allowed here";
		}
	}



	# bodge: works around differences with an older version of the specs
	$param{url} = $param{url1} if !defined $param{url} and defined $param{url1};
	$param{ip} = $param{ip1} if !defined $param{ip} and defined $param{ip1};

	if ( defined $param{url} or defined $param{ip} )
	{
		# limit update rate by remembering the cache...
		my %updateip = loadhashhash "$prefix-updateip";
		foreach my $ip ( keys %updateip )
		{
			if ( $updateip{$ip}{time} < $time - $updatelimit*60 )
			{
				delete $updateip{$ip};
				next;
			}

			if ( $ENV{REMOTE_ADDR} eq $ip )
			{
				statlog $day, "update_block_ratelimit";

				print "OK\n";
				print "WARNING: update denied: $ENV{REMOTE_ADDR} has sent updates in the last $updatelimit minutes\n";
				exit;
			}
		}
		$updateip{$ENV{REMOTE_ADDR}}{time} = $time;

		savehashhash "$prefix-updateip", %updateip;


		print "OK\n";

		if ( defined $param{ip} )
		{
			my $ipport = $param{ip};

			if ( $ipport =~ m{^(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}):(\d{1,5})$} )
			{
				my ( $ip, $port ) = ( $1, $2 );

				if ( $port < 1 or $port > 65535 )
				{
					statlog $day, "update_block_badport";
					error "port $port out of range";
				}
				my $iptype = getiptype $ip;

				# TODO: allow 'private' addresses in certain situations??
				unless ( $iptype eq 'public' )
				{
					statlog $day, "update_block_badip";
					error "ip $ip invalid ($iptype)";
				}
				
				unless( $client eq "MUTE" ) {
				    error "client type $client invalid";
				}
				addhost "gnutella", $ipport, $client;

			}
			else
			{
				print "WARNING: invalid format ip parameter\n";
			}
		}

		if ( defined $param{url} )
		{
			my $url = $param{url};

			# bodge to change %7e to ~
			$url =~ s{%7[eE]}{~}g;

			if ( $url =~ m{^http://[a-zA-Z0-9.-]{1,200}(:\d{1,5})?/[a-zA-Z0-9/.~_-]{0,250}$} )
			{
				addurl 1, $url;		# add url to the gwebcache v1 list...
			}
			else
			{
				print "WARNING: invalid format/character in url\n";
			}
		}

		statlog $day, "update";
	}
	elsif ( defined $param{hostfile} )
	{

		my @hosts = gethosts $network;		##loadarrayhash "$prefix-hosts";

#		@hosts = pickhosts $client, @hosts;

		my $testclient = $client eq 'unknown' ? 'not set' : $client;

		my $outcount = 0;

		foreach my $item ( @hosts )
		{
			print "${$item}{ipport}\n";
		}

		# hack: the v1 cache specs allows caches to return no lines, but some clients
		# assume this is an error. let's always send something, even if it's useless
		# NOTE: this only happens when the cache has no hosts
		if ( @hosts == 0 )
		{
			print "0.0.0.0:0\n";	# an invalid IP/port but should match the format clients are expecting
		}

		statlog $day, "hostfile";
	}
	elsif ( defined $param{urlfile} )
	{
		my @urls = geturls 1;	# load gwebcache v1 list

		@urls = pickurls @urls;

		foreach my $item ( @urls )
		{
			print "${$item}{url}\n";
		}

		# hack: the v1 cache specs allows caches to return no lines, but some clients
		# assume this is an error. let's always send something, even if it's useless
		# NOTE: this only happens when the cache has no valid URLs
		if ( @urls == 0 )
		{
			print "http://0.0.0.0:0/\n";	# an invalid IP/port but should match the format clients are expecting
		}

		statlog $day, "urlfile";
	}
	elsif ( defined $param{ping} )
	{
		print "PONG perlgcache/$cacheversion\n";

		statlog $day, "ping";
	}
	elsif ( defined $param{statfile} )
	{
		if ( $enablestats )
		{
			my %stats = loadhashhash "$prefix-stats";

			my ( $totalreq, $dayreq, $dayupd ) = ( 0, 0, 0 );

			my $statday = $day - 1;

			# type1: count all requests for the requests figure...
			foreach my $statitem ( keys %{$stats{'total'}} )
##			# type2: count just hostfile and urlfile requests
##			foreach my $statitem ( 'hostfile', 'urlfile' )
			{
				$dayreq += $stats{$statday}{$statitem} if defined $stats{$statday};
				$totalreq += $stats{'total'}{$statitem};
			}

			$dayupd += $stats{$statday}{'update'} if defined $stats{$statday};
			$dayupd += $stats{$statday}{'v2_update'} if defined $stats{$statday};

			print $totalreq."\n";
			# NOTE: the statfile=1 specs ask for hour stats, but we only store day stats... fake them...
			print int($dayreq/24)."\n";
			print int($dayupd/24)."\n";

		}
		else
		{
			print "NOTE: statfile not supported\n";
		}

		statlog $day, "statfile";
	}
	else
	{
		statlog $day, "badcommand";
		error "unknown command";
	}

}



sub dov2request
{
	my ( %param ) = @_;

	my ( $client, $version ) = getclientandversion %param;

	my $network = 'gnutella';	## NOTE: default net name - supplying no net= param is the same as sending this value
	$network = lc $1 if defined $param{net} and $param{net} =~ m/^([a-zA-Z0-9._-]{1,20})$/;
	# (NOTE: above restrictions are the same as I use for the client string - no particular reason for this)

	if ( 1 )	# always enforce client parameter for v2 requests
	{
		if ( $client =~ m/^unknown/ )	# just check for client
		{
			statlog $day, "v2_block_anon";

			error "V2 Anonymous clients not allowed here";
		}
	}


	# OK - the v2 specs say that a cache must always return something - returning nothing is an error
	# they also specify a response to a ping but not the query to make!
	# there's also the fact that unwanted info can always be returned

	# so... - let's always output our ping line...
	print "i|pong|perlgcache/$cacheversion\n";


	my @actions;

	if ( defined $param{ping} )
	{
		push @actions, "ping";
	}

	if ( defined $param{update} )
	{
		# limit update rate by remembering the cache...
		my %updateip = loadhashhash "$prefix-updateip";
		foreach my $ip ( keys %updateip )
		{
			if ( $updateip{$ip}{time} < $time - $updatelimit*60 )
			{
				delete $updateip{$ip};
				next;
			}

			if ( $ENV{REMOTE_ADDR} eq $ip )
			{
				statlog $day, "update_block_ratelimit";

				print "i|update|WARNING|You came back too soon\n";
				exit;
			}
		}
		$updateip{$ENV{REMOTE_ADDR}}{time} = $time;

		savehashhash "$prefix-updateip", %updateip;



		if ( defined $param{ip} )
		{
			my $ipport = $param{ip};

			if ( $ipport =~ m{^(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}):(\d{1,5})$} )
			{
				my ( $ip, $port ) = ( $1, $2 );

				if ( $port < 1 or $port > 65535 )
				{
					statlog $day, "v2_update_block_badport";

					print "i|update|WARNING|Rejected IP\n";
					print "d|update|ip|port $port out of range\n";
					return;
				}

				my $iptype = getiptype $ip;

				# TODO: allow 'private' addresses in certain situations??
				unless ( $iptype eq 'public' )
				{
					statlog $day, "v2_update_block_badip";

					print "i|update|WARNING|Rejected IP\n";
					print "d|update|ip|ip $ip isn't public - $iptype\n";
				}

				addhost $network, $ipport, $client;

				print "i|update|OK\n";
				print "d|update|ip|ip=$ip:$port net=$network accepted\n";

			}
			

		}


		if ( defined $param{url} )
		{
			my $url = $param{url};

			# bodge to change %7e to ~
			$url =~ s{%7[eE]}{~}g;

			if ( $url =~ m{^http://[a-zA-Z0-9.-]{1,200}(:\d{1,5})?/[a-zA-Z0-9/.~_-]{0,250}$} )
			{
				addurl 2, $url, $network;		# add url to the gwebcache v2 list...
				print "i|update|OK\n";
				print "d|update|url|url=$url added to gwc2 list\n";

			}
			else
			{
				statlog $day, "v2_update_block_badurl";

				print "i|update|WARNING|Rejected URL\n";

				return;
			}
		}

		push @actions, "update";
	}


	if ( $param{get} )
	{
		# first, do the hosts list...
		my @hosts = gethosts $network;

#		@hosts = pickhosts $client, @hosts;

		foreach my $item ( @hosts )
		{
			my $age = $time - ${$item}{time};
			$age = 0 if $age < 0;
			print "h|${$item}{ipport}|$age|${$item}{client}\n";
		}


		# now do the urls...

		my @urls = geturls 2, $network;

		@urls = pickurls @urls;

		foreach my $item ( @urls )
		{
			my $age = $time - ${$item}{time};
			$age = 0 if $age < 0;
			print "u|${$item}{url}|$age\n";
		}

		push @actions, "get";
	}

	if ( defined $param{support} )
	{
		my @networks = getnetworks;

		foreach my $network ( @networks )
		{
			print "i|SUPPORT|$network\n";
		}
		print "i|SUPPORT|*\n";	# show we support any unknown network
	}

	statlog $day, "v2_".(join "_", sort @actions);

}




sub dorequest
{
	# globals: read time once - to avoid possible race conditions
	$time = time;
	$day = int ( $time / (24*60*60) );


	unless ( defined $ENV{REQUEST_METHOD} and $ENV{REQUEST_METHOD} eq 'GET' )
	{
		print "Content-type: text/plain\n";
		print "Cache-control: no-cache\n";
		print "Connection: close\n";
		print "\n";
		error "only GET requests supported" 
	}



	my %param = paramdecode $ENV{QUERY_STRING};



	if ( scalar %param eq "0" or defined $param{browse} )
	{
		dohtmlpage %param;
		exit;
	}



	print "Content-type: text/plain\n";
	print "Cache-control: no-cache\n";
	print "Connection: close\n";
	print "\n";


	# to avoid multiple paths to this script (eg .../perlgcache.cgi and .../perlgcache.cgi/)
	if ( defined $ENV{PATH_INFO} and $ENV{PATH_INFO} ne '' )
	{
		statlog $day, "block_badpath";
		error "invalid path to script (PATH_INFO = $ENV{PATH_INFO})";
	}


	# block http/0.9 requests
	# (there's one anonymous client which just sends "GET http://host/path/script\r\n")
	if ( 1 )
	{
		if ( defined $ENV{SERVER_PROTOCOL} and $ENV{SERVER_PROTOCOL} eq 'HTTP/0.9' )
		{
			statlog $day, "block_http0.9";
			error "HTTP/0.9 requests not supported. read RFC2616 or at least RFC1945";
		}
	}


	if ( $requestlimitcount and $requestlimittime )
	{
		my %requestip = loadhashhash "$prefix-requestip";

		# first, remove old entries...
		foreach my $ip ( keys %requestip )
		{
			if ( $requestip{$ip}{time} < $time - $requestlimittime*60 )
			{
				delete $requestip{$ip};
				next;
			}
		}

		# add a blank entry if required...
		unless ( defined $requestip{$ENV{REMOTE_ADDR}} )
		{
			$requestip{$ENV{REMOTE_ADDR}}{count} = 0;
			$requestip{$ENV{REMOTE_ADDR}}{time} = $time;
		}
		# increment the counter...
		$requestip{$ENV{REMOTE_ADDR}}{count}++;

		# and save it back...
		savehashhash "$prefix-requestip", %requestip;

		if ( $requestip{$ENV{REMOTE_ADDR}}{count} > $requestlimitcount )
		{
			statlog $day, "block_requestrate";

			# start adding delays to high request rate clients. the theory
			# is that even abusive clients still wait for a request to complete
			# before making another request
			sleep 5 if $requestip{$ENV{REMOTE_ADDR}}{count} > $requestlimitcount*2;
			sleep 10 if $requestip{$ENV{REMOTE_ADDR}}{count} > $requestlimitcount*3;
			sleep 15 if $requestip{$ENV{REMOTE_ADDR}}{count} > $requestlimitcount*5;
			# if over 10 times the standard limit, then exit without a message (saves a few bytes)
			exit if $requestip{$ENV{REMOTE_ADDR}}{count} > $requestlimitcount*10;

			error "request rate of $requestlimitcount requests every $requestlimittime minutes exceeded for $ENV{REMOTE_ADDR}\n";
		}

	}



	if ( $enablev2requests and
		(
		 !$enablev1requests or
		 defined $param{get} or
		 defined $param{update} or
		 defined $param{net} or
		 (defined $param{ping} and $param{ping} eq '2')
		)
	   )
	{
		dov2request %param;
	}
	else
	{
		dov1request %param;
	}



}


# regular CGI
dorequest;


# NOTE: not working yet - there's several cases where the above code does an 'exit' instead
# of a 'return', so it won't work well...
##FCGI### FastCGI
##FCGI##use FCGI;
##FCGI##my $fcgi = FCGI::Request();
##FCGI##while ( $fcgi->Accept() >= 0 )
##FCGI##{
##FCGI##	dorequest;
##FCGI##	$fcgi->Finish();
##FCGI##	# exit if script file changes...
##FCGI##	last if -M $ENV{SCRIPT_FILENAME} < 0;
##FCGI##}

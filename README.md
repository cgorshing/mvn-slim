# Project Title

This is to solve an itch/need of having a maven repository manager that is
light on memory, is plain text driven, and limited frills. Although memory is
definitely cheap, still having a repo manager being sensitive to memory is
needed so they can be ran within a vagrant vm, or AWS instance and not require
too many resources. This will not have many of the features others repo
managers have, that is not the goal of this project.

Another item beleived to be beneficial is the provisioning of servers using
tools like Terraform and Chef. Some of the existing repo managers are "designed
primarily for configuration from the UI". This project takes a different
approach, similar to how Apache's httpd is configured, being plain text driven.
Of course an added benefit of this is more easily adding server configuration
to source control as well.

## Getting Started

As of now, this is being built on MacOSX. Other platforms will be tested, but
nothing as of yet.

### Prerequisites

Nothing is known of any additional libraries/dependencies.

```
gcc main.c thread_serve.c
```

### Installing

No installation yet, just build it.
A step by step series of examples that tell you have to get a development env running

Say what the step will be

```
gcc main.c thread_serve.c -o wrench
```

And repeat

```
./wrench -r /some/directory/path
```

Then use curl to test it

```
curl -v http://localhost:8080/filename
```

## Running the tests

Sniff sniff, no tests. I'm still figuring out other things.

### Break down into end to end tests

Explain what these tests test and why

```
Give an example
```

### And coding style tests

Explain what these tests test and why

```
Give an example
```

## Deployment

Copy executable out and go.

## Built With

* [gcc](https://gcc.gnu.org/) - The compiler used
* [pthreads](https://computing.llnl.gov/tutorials/pthreads/) - Threading model

## Contributing

Please read [CONTRIBUTING.md](https://github.com/cgorshing/wrench/CONTRIBUTING.md) for details on our code of conduct, and the process for submitting pull requests to us.

## Versioning

We use [SemVer](http://semver.org/) for versioning. For the versions available, see the [tags on this repository](https://github.com/cgorshing/wrench/tags). 

## Authors

* **Chad Gorshing** - *Initial work* - [cgorshing](https://github.com/cgorshing)

See also the list of [contributors](https://github.com/cgorshing/wrench/contributors) who participated in this project.

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

## Acknowledgments

* Hat tip to anyone who's code was used
* Inspiration
* etc


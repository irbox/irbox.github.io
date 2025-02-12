import Image from "next/image"
import Link from "next/link"
import { Button } from "@/components/ui/button"
import { ArrowRight } from "lucide-react"

export default function Home() {
  return (
    <>
      <section className="relative">
        <div className="overflow-hidden">
          <div className="mx-auto max-w-7xl px-6 pb-32 pt-36 sm:pt-60 lg:px-8 lg:pt-32">
            <div className="mx-auto max-w-2xl gap-x-14 lg:mx-0 lg:flex lg:max-w-none lg:items-center">
              <div className="w-full max-w-xl lg:shrink-0 xl:max-w-2xl">
                <h1 className="font-heading text-4xl font-bold tracking-tight text-foreground sm:text-6xl">
                  The Eastern Apostolic Seat of Adi Shankaracharya
                </h1>
                <p className="relative mt-6 text-lg leading-8 text-muted-foreground sm:max-w-md lg:max-w-none">
                  Govardhan Math, established by Adi Shankaracharya, is one of the four cardinal monasteries preserving
                  and promoting Sanatana Dharma through spiritual guidance and education.
                </p>
                <div className="mt-10 flex items-center gap-x-6">
                  <Button asChild size="lg">
                    <Link href="/about">
                      Learn More
                      <ArrowRight className="ml-2 h-4 w-4" />
                    </Link>
                  </Button>
                  <Button variant="outline" size="lg" asChild>
                    <Link href="/contact">Contact Us</Link>
                  </Button>
                </div>
              </div>
              <div className="mt-14 flex justify-end gap-8 sm:-mt-44 sm:justify-start sm:pl-20 lg:mt-0 lg:pl-0">
                <div className="ml-auto w-44 flex-none space-y-8 pt-32 sm:ml-0 sm:pt-80 lg:order-last lg:pt-36 xl:order-none xl:pt-80">
                  <div className="relative">
                    <Image
                      src="https://sjc.microlink.io/9IJgQJuALINvLK6IfkXjmE72lN8EPXz-D0vRBZmOfQLQkSK8BzAHopqdAhq9OQsO0rzEGtj99pdpURJrpT8-UA.jpeg"
                      alt="Govardhan Math"
                      className="aspect-[2/3] w-full rounded-xl bg-gray-900/5 object-cover shadow-lg"
                      width={176}
                      height={264}
                    />
                    <div className="pointer-events-none absolute inset-0 rounded-xl ring-1 ring-inset ring-gray-900/10" />
                  </div>
                </div>
                <div className="mr-auto w-44 flex-none space-y-8 sm:mr-0 sm:pt-52 lg:pt-36">
                  <div className="relative">
                    <Image
                      src="/placeholder.svg"
                      alt="Temple architecture"
                      className="aspect-[2/3] w-full rounded-xl bg-gray-900/5 object-cover shadow-lg"
                      width={176}
                      height={264}
                    />
                    <div className="pointer-events-none absolute inset-0 rounded-xl ring-1 ring-inset ring-gray-900/10" />
                  </div>
                  <div className="relative">
                    <Image
                      src="/placeholder.svg"
                      alt="Sacred rituals"
                      className="aspect-[2/3] w-full rounded-xl bg-gray-900/5 object-cover shadow-lg"
                      width={176}
                      height={264}
                    />
                    <div className="pointer-events-none absolute inset-0 rounded-xl ring-1 ring-inset ring-gray-900/10" />
                  </div>
                </div>
              </div>
            </div>
          </div>
        </div>
      </section>

      <section className="mx-auto max-w-7xl px-6 lg:px-8 py-24 sm:py-32">
        <div className="mx-auto max-w-2xl lg:text-center">
          <h2 className="font-heading text-3xl font-bold tracking-tight sm:text-4xl">Preserving Ancient Wisdom</h2>
          <p className="mt-6 text-lg leading-8 text-muted-foreground">
            Through centuries of spiritual tradition, Govardhan Math has been a beacon of Vedantic knowledge and
            Sanskrit learning.
          </p>
        </div>
        <div className="mx-auto mt-16 max-w-2xl sm:mt-20 lg:mt-24 lg:max-w-none">
          <dl className="grid max-w-xl grid-cols-1 gap-x-8 gap-y-16 lg:max-w-none lg:grid-cols-3">
            <div className="flex flex-col">
              <dt className="font-heading text-lg font-semibold leading-7">Spiritual Guidance</dt>
              <dd className="mt-4 flex flex-auto flex-col text-base leading-7 text-muted-foreground">
                <p className="flex-auto">
                  Offering spiritual direction and guidance through traditional Vedantic teachings and practices.
                </p>
              </dd>
            </div>
            <div className="flex flex-col">
              <dt className="font-heading text-lg font-semibold leading-7">Sanskrit Education</dt>
              <dd className="mt-4 flex flex-auto flex-col text-base leading-7 text-muted-foreground">
                <p className="flex-auto">
                  Preserving and promoting Sanskrit education through traditional Gurukula system.
                </p>
              </dd>
            </div>
            <div className="flex flex-col">
              <dt className="font-heading text-lg font-semibold leading-7">Cultural Heritage</dt>
              <dd className="mt-4 flex flex-auto flex-col text-base leading-7 text-muted-foreground">
                <p className="flex-auto">
                  Maintaining and promoting India's rich cultural heritage through various programs and initiatives.
                </p>
              </dd>
            </div>
          </dl>
        </div>
      </section>
    </>
  )
}
